#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

pthread_rwlock_t lock;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";
static const char *httptype_hdr = "HTTP/1.0\r\n";

struct http_request
{
    char * method; // "GET"
    char * url; // "http://localhost:12345/home.html"
    char * httptype; // "HTTP/1.0"
    char * host; // "localohst:12345"
    char * user_agent; 
    char * conn;
    char * proxy_conn;
};

/* ---------------------------------- cache part ---------------------------------*/

/* cache consists of three component,
 * 1. maximum age : used for LRU, used and updated when reading cache or writing cache is occurred
 * 2. total sizes of data buffer : also used when deciding whether eviction should be done or not.
 * 3. head of doubly linked list : stores the whole datas and matadatas
 *
 * Each node in the doubly linked list  has
 * 1. age : used for LRU
 * 2. url string : used for checking cache hit
 * 3. data buffer : caching
 * 4. data_size : used in order to send the data buffer easily
 * 5. prev : link for previous node in the linked list
 * 6. next : link for next node in the linked list
 *
 * For convenience, I used linear search.
 *
 *
 * IMPORTANT!
 * I did not managed the synchronization between multi-threads in these functions.
 * Synchronization will be done by the do_proxy(), which is the only caller function of these cache functions.
 */
typedef struct NODE
{
    unsigned int age; // used for LRU
    unsigned int data_size;
    char * url;
    void * data;
    struct NODE * prev;
    struct NODE * next;
} node;

typedef struct
{
   unsigned int size;
   unsigned int max_age;
   node * head;
} Cache;

static Cache cache;
static Cache * cachep = &cache;

// initializing the given cache.
void init_cache(Cache * c)
{
    if(c == NULL)
    {
        fprintf(stderr, "init_cache : tried to initialize the empty cache\n");
        exit(1);
    }
    c->size = 0;
    c->max_age = 0;
    c->head = NULL;
}

// deleting the given node from the cache.
// unlinking, size controling, and freeing will be done.
void del_node(Cache * c, node * nd)
{
    if(nd == NULL)
        return;
    if(nd->prev != NULL)
        nd->prev->next = nd->next;
    if(nd->next != NULL)
        nd->next->prev = nd->prev;
    if(c->head == nd)
        c->head = nd->next;

    c->size -= nd->data_size;

    Free(nd->url);
    Free(nd->data);
    Free(nd);
}

// deleting the cache.
// it will be done after all jobs finished.
void del_cache(Cache * c)
{
    node * cursor = NULL;

    if(c == NULL)
    {
        fprintf(stderr, "del_cache : tried to delete the empty cache\n");
        exit(1);
    }

    for(cursor = c->head; cursor != NULL; cursor = cursor->next)
        del_node(c, cursor);
}

// search the Least Recently Used(Read or Write) node in the given cache.
// if there is no node in Cache c, return NULL.
// else, return the least recently used node, based on the age of each node.
node * LRU_search(Cache * c)
{
    node * cursor = NULL;
    node * LRU_node = NULL;
    unsigned int min_age = 0xFFFFFFFF;

    if(c == NULL)
        return NULL;

    for(cursor = c->head; cursor != NULL; cursor = cursor->next)
    {
        if(cursor->age <= min_age)
        {
            min_age = cursor->age;
            LRU_node = cursor;
        }
    }
    return LRU_node;
}

// write the given data with the given url(works as a tag)
void write_in_cache(Cache * c, char * url, void * data, unsigned int data_size)
{
    node * nd;
    int url_size = strlen(url) + 1; // considering null byte.
    if(c == NULL)
    {
        fprintf(stderr, "write_in_cache : tried to write into the empty cache.\n");
        exit(1);
    }

    if(data_size > MAX_OBJECT_SIZE)
    {
        fprintf(stderr, "write_in_cache : tried to write data over the size MAX_OBJECT_SIZE, it should be skipped in caller side.\n");
        exit(1);
    }

    /* if cache does not have an enough left space, evict the LRU nodes until the cache has an enough space. */
    while(c->size + data_size > MAX_CACHE_SIZE)
    {
        del_node(c, LRU_search(c));
    }

    nd = (node *)Malloc(sizeof(node));
    nd->url = (char *)Malloc(url_size);
    nd->data = Malloc(data_size);

    strncpy(nd->url, url, url_size);
    memcpy(nd->data, data, data_size);

    nd->data_size = data_size;

    // update the max_age of the cache, and set the age of node appropriately.
    c->max_age += 1;
    nd->age = c->max_age;

    // insert the new-made node in the head of the doubly linked list.
    nd->next = c->head;
    if(nd->next != NULL)
        nd->next->prev = nd;
    c->head = nd;

    // update the total data size of the cache.
    c->size += data_size;
}

// returns node * that corresponds to the given url.
// if not found, return NULL.
node * urlmatch_search(Cache * c, char * url)
{
    node * cursor;
    if(c == NULL)
    {
        fprintf(stderr, "search : tried to search the empty cache.\n");
        exit(1);
    }

    for(cursor = c->head; cursor != NULL; cursor = cursor->next)
    {
        if(!strcmp(cursor->url, url))
        {
            fprintf(stdout, "search : matching url %s found!\n", url);
            return cursor;
        }
    }

    fprintf(stdout, "search : matching url %s not found...\n", url);
    return NULL;
}
/* ---------------------------------- END of cache part ---------------------------------*/

void * thread(void * connfdp);
void do_proxy(int connfdp);
void parse_url(char * url, char * host, char * port, char * path);
void httpreq_to_str(struct http_request * req, char * dest);
void init_httpreq(struct http_request * req, char * method, char * url, char * host);


void close_wrapper(int fd) {
    if (close(fd) < 0)
        printf("Error closing file.\n");
}

int main(int argc, char *argv[])
{
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    // ignore sigpipes
    signal(SIGPIPE, SIG_IGN);

    // initialize reader/writer lock for cache
    pthread_rwlock_init(&lock, NULL);

    // establish client port (default: 29094)
    if (!argv[1]){
        printf("Missing command line port number\n");
        return -1;
    }

    // establish listening file
    listenfd = open_listenfd(argv[1]);
    clientlen = sizeof(clientaddr);

    init_cache(cachep);

    if (listenfd < 0)
        printf("open_listenfd failed.\n");
    else {
        while (1) {
            // when a client connects, spawn a new thread to handle it.
            connfdp = Malloc(sizeof(int));
            *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
            if (connfdp < 0) {
                printf("Accept failed.\n");
                Free(connfdp);
            }
            else {
                // do something
                Pthread_create(&tid, NULL, thread, connfdp);
            }
        }
    }
    del_cache(cachep);
    close_wrapper(listenfd);
    pthread_rwlock_destroy(&lock);
    return 0;
}

// what each thread does.
void * thread(void * connfdp)
{
    int connfd = *((int *)connfdp); // connfd with client
    Pthread_detach(pthread_self());
    Free(connfdp);
    do_proxy(connfd);
    close_wrapper(connfd);
    return NULL;
}

 
/* for given connfd(with client),
 * do_proxy receive the request from client, parse the request,
 * check whether it can use the web cache,
 * if necessary, 
 * send the client's request to the server and receive the response from server,
 * forward the very response to the client with storing the response into its own web cache.
 *
 * IMPORTANT!
 * Synchronization for cache was done in here.
 */
void do_proxy(int connfd)
{
    int i, port_given;
    rio_t rio1; // rio_t for reading client's request.
    char method[16];
    char url[MAXLINE];
    char httptype[16];
    char host[MAXLINE]; // this host do not contain port.
    char port[8]; // string "80" -> decimal 80.
    char path[MAXLINE];
    char * merged_host; // this host will contain port if exist.
    char send_str[MAXLINE];
    struct http_request send_req;
    node * searched_node; // used when cache hit occurs.

    int serverfd; // fd connected with server
    rio_t rio2; // rio_t for reading server's response.
    int total_amount = 0;
    char buf[MAXLINE]; // temporary buffer that stores the each response line from server.
    char total_response[MAX_OBJECT_SIZE]; // array that will store whole response datas from server.

    Rio_readinitb(&rio1, connfd);

    // parse HTTP header into method, url, and httptype(version).
    for(i = 0; i < MAXLINE; ++i)
    {
        if(Rio_readnb(&rio1, method + i, 1) != 1) break;
        if(method[i] == ' ')
        {
            method[i] = '\0';
            fprintf(stdout, "parsed method : %s\n", method);
            break;
        }
    }
    for(i = 0; i < MAXLINE; ++i)
    {
        if(Rio_readnb(&rio1, url + i, 1) != 1) break;
        if(url[i] == ' ')
        {
            url[i] = '\0';
            fprintf(stdout, "parsed url : %s\n", url);
            break;
        }
    }
    for(i = 0; i < MAXLINE; ++i)
    {
        if(Rio_readnb(&rio1, httptype + i, 1) != 1) break;
        if(httptype[i] == ' ' || httptype[i] == '\n')
        {
            if(i > 0 && httptype[i-1] == '\r')
            {
                httptype[i] = '\0';
                httptype[i-1] = '\0';
            }
            else
            {
                fprintf(stderr, "wrong header format\n");
                exit(1);
            }
            fprintf(stdout, "parsed httptype : %s\n", httptype);
            break;
        }
    }

    fprintf(stdout, "trying to search the appropriate data in the cache with url : %s\n", url);
    pthread_rwlock_wrlock(&lock); // significant read section starts
    if((searched_node = urlmatch_search(cachep, url)) != NULL)
    {
        pthread_rwlock_unlock(&lock); // significant read section finished

        /* cache hit! */
        pthread_rwlock_wrlock(&lock); // significant read section starts

        fprintf(stdout, "CACHE HIT for url %s\n", url);
        fprintf(stdout, "********Data in searched_node********\n");
        fprintf(stdout, "%s\n", searched_node->data);
        fprintf(stdout, "*************************************\n");
        Rio_writen(connfd, searched_node->data, searched_node->data_size);

        pthread_rwlock_unlock(&lock); // significant read section finished

        /* update the maximum age and the age of read node. */
        pthread_rwlock_rdlock(&lock); // significant write section starts

        cachep->max_age += 1;
        searched_node->age = cachep->max_age;

        pthread_rwlock_unlock(&lock); // significant write section finished
        return;
    }
    pthread_rwlock_unlock(&lock); // significant read section finished

    // parse the host info and port info from url and store it in the char host[], char port[].
    parse_url(url, host, port, path);
    printf("parsed host : %s, port : %s, path : %s\n", host, port, path);

    // merge the host info and port info. It will be used when I make a http_request form.
    merged_host = (char *)Malloc(strlen(host) + strlen(port) + 2); // 1 for ':', 1 for '\0'
    strcat(merged_host, host);
    if(strlen(port) != 0)
    {
        port_given = 1;
        strcat(merged_host, ":");
        strcat(merged_host, port);
    }
    else
        port_given = 0;

    // make a request string which will be sent to the server.
    init_httpreq(&send_req, method, path, merged_host);
    httpreq_to_str(&send_req, send_str);
    fprintf(stdout, "********sending request********\n");
    fprintf(stdout, "%s\n", send_str);
    fprintf(stdout, "*******************************\n");


    // if port is not given, set it to the "80" since it's a default port for http.
    if(!port_given)
    {
        port[0] = '8';
        port[1] = '0';
        port[2] = '\0';
    }

    /* send a clients requests to the server & getting an response from sever */
    serverfd = Open_clientfd(host, port);
    Rio_readinitb(&rio2, serverfd);
    Rio_writen(serverfd, send_str, strlen(send_str));

    /*
     * Receive a response of the server, and forward it to the client.
     *
     * I'll first read the response line by line, move them into the buf[MAXLINE] temporarily with forwarding to the client.
     * Then, accumulate them in total_response.
     * Accumulated total_response will be used to store the given response in the cache.
     */
    memset(total_response, 0, MAX_OBJECT_SIZE);
    while((i = Rio_readlineb(&rio2, buf, MAXLINE)) > 0)
    {
        if(total_amount + i<= MAX_OBJECT_SIZE)
        {
            // before memcpy, checking the current read amount should be done so that memcpy() does not overlap the significant data in the stack.
            memcpy(total_response + total_amount, buf, i);
        }
        total_amount += i;

        // forward responses.
        Rio_writen(connfd, buf, i);
    }
    Close(serverfd);
    /* end of interacting with server */

    if(total_amount <= MAX_OBJECT_SIZE)
    {
        // the given url was not in cache & response is able to be stored in cache. -> store in the cache
        fprintf(stdout, "writing the data for url %s in cache...\n", url);
        pthread_rwlock_rdlock(&lock); // significant write section starts

        write_in_cache(cachep, url, total_response, total_amount);
        // whether evict some datas or not will be determined by write_in_cache()

        pthread_rwlock_unlock(&lock); // significant write section finished
    }

    //Close(connfd); this is done after returned, in main()

    Free(merged_host);
}

// get url string and char * for host and port(both for storing)
// parse a host, port, and path string SEPARATELY from the url string, 
// and store them in the given argument pointers(host, port, path)
void parse_url(char * url, char * host, char * port, char * path)
{
    int port_exist = 0;
    int rest_len = 0;
    int i;
    char * cursor = url;

    if(url == NULL || host == NULL || path == NULL) return;
    rest_len = strlen(url);

    /*
     * url looks like
     * http://hostname.host.name/something/something
     * 1. First part should be "http://". If not, it's a wrong url.
     * 2. Next part until the first '/' is the host. It can include port number also.  ex) hostname.host.name
     * 3. Ignore the rest string part. ex)/something/something
     */

    if(rest_len <= 7 || strncmp(url, "http://", 7))
    {
        // if url does not start with "http://", error
        fprintf(stderr, "parse_url : wrong url, rest_len : %d, it does not start with \"http://\", instead, %s\n", rest_len, url);
        exit(1);
    }

    cursor += 7;
    rest_len -= 7;

    for(i = 0; i < rest_len; ++i) // escape when i = rest_len so that cursor[i] == '\0' OR cursor[i] == ':' / '/'
    {
        host[i] = cursor[i];
        if(cursor[i] == ':')
        {
            port_exist = 1;
            break;
        }
        if(cursor[i] == '/') break;
    }
    host[i] = '\0';

    cursor += i;
    rest_len -= i;

    if(port_exist)
    {
        for(i = 0; i < rest_len; ++i)
        {
            if(i == 0 && cursor[i] == ':') continue;
            if(i > 8)
            {
                fprintf(stderr, "parse_url : port is longer than maximum port length 8\n");
                exit(1);
            }
            if(cursor[i] == '/') break;
            if(cursor[i] < 0x30 || cursor[i] > 0x39)
            {
                fprintf(stderr, "parse_url : port includes non-numeric character %c\n", cursor[i]); // only allowed non-numerics are ':' and '/'
                exit(1);
            }
            port[i - 1] = cursor[i];
        }
        port[i] = '\0';

        cursor += i;
        rest_len -= i;
    }

    path[0] = '/'; // path starts with '/'.
    for(i = 0; i < rest_len; ++i)
    {
        if(i == 0 && cursor[i] == '/') continue;
        path[i] = cursor[i];
    }
    path[i] = '\0';
}

// convert a given http_request form into the appropriate string form, and store the converted result into dest.
void httpreq_to_str(struct http_request * req, char * dest)
{
    strcat(dest, req->method);
    strcat(dest, " ");
    strcat(dest, req->url);
    strcat(dest, " ");
    strcat(dest, req->httptype); // "\r\n" already exist
    strcat(dest, "Host: ");
    strcat(dest, req->host);
    strcat(dest, "\r\n");
    strcat(dest, req->user_agent);
    strcat(dest, req->conn);
    strcat(dest, req->proxy_conn);
    strcat(dest, "\r\n"); // last empty line "\r\n"
}

// initialize the given http_request variable with given method, url, host strings and other static const strings such as httptype_hdr and user_agent_hdr.
void init_httpreq(struct http_request * req, char * method, char * url, char * host)
{
    req->method = method;
    req->url = url;
    req->httptype = httptype_hdr; // with "\r\n"
    req->host = host;
    req->user_agent = user_agent_hdr; // with "\r\n"
    req->conn = conn_hdr; // with "\r\n"
    req->proxy_conn = proxy_conn_hdr; // with "\r\n"
}
