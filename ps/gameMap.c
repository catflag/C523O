#include <stdio.h>
#include <stdlib.h>

#define MAXCITY 100000
//#define MAXLINE 300000
#define DEBUG 0

char ** adj;
int dp_res[MAXCITY] = {0, };
int degree[MAXCITY] = {0, };
int total_city = 0;



//int dp(int citynum, bool visited[MAXCITY])
int dp(int citynum, char * visited)
{
    char * visited_copy = (char *)malloc(total_city * sizeof(char));
    int max_len = 0;
    int temp_res = 0;
    int i;
    
    for(i = 0; i < total_city; ++i)
    {
        visited_copy[i] = visited[i];
    }

    if(visited[citynum]) // ban -> true
    {
        // if already banned -> error
        fprintf(stderr, "dp : %d, already visited but came into dp\n", citynum);
        return -1;
    }

    if(dp_res[citynum] > 0)
    {
        if(DEBUG) printf("M : dp_res[%d] = %d\n",citynum, dp_res[citynum]); 
        free(visited_copy);
        return dp_res[citynum];
    }

    visited_copy[citynum] = 1;

    // make the available adj (which is adj AND has higher degree.

    for(i = 0; i < total_city; ++i)
    {
        if(adj[citynum] != NULL && adj[citynum][i] && !visited[i] && degree[i] > degree[citynum])
        {
            temp_res = dp(i, visited_copy);
            if(temp_res > max_len)
            {
                max_len = temp_res;
            }
        }
    }

    dp_res[citynum] = max_len + 1;
    if(DEBUG) printf("non-M dp_res[%d] = %d\n", citynum, dp_res[citynum]);
    free(visited_copy);
    return max_len + 1;
}


int main()
{
    int n = 0;
    int m = 0;
    int max_len = 0;
    int temp1 = -1;
    int temp2 = -1;
    int i, j;
    char * visited;
    int * lhs_buf;
    int * rhs_buf;

    scanf("%d %d", &n, &m);

    total_city = n;

    adj = (char **)malloc(n * sizeof(char *));
    visited = (char *)malloc(n * sizeof(char));
    lhs_buf = (int *)malloc(m * sizeof(int));
    rhs_buf = (int *)malloc(m * sizeof(int));

    for(i = 0; i < m; ++i)
    {
        scanf("%d %d", &temp1, &temp2);

        if(temp1 == -1 || temp2 == -1)
        {
            fprintf(stderr, "scanf failed\n");
            exit(1);
        }
        lhs_buf[i] = temp1;
        rhs_buf[i] = temp2;

        degree[temp1] += 1;
        degree[temp2] += 1;

        if(temp1 == temp2)
            degree[temp1] -= 1;
    }

    for(i = 0; i < n; ++i)
    {
        adj[i] = NULL;
        if(degree[i] > 0)
        {
            adj[i] = (char *)malloc(n * sizeof(char));
        }
    }
    
    /*
    for(i = 0; i < MAXCITY; ++i)
    {
        for(j = 0; j < MAXCITY; ++j)
        {
            adj[i][j] = 0;
        }
        //adj[i].assign(MAXCITY, false);
        degree[i] = 0;
        dp_res[i] = 0;
    }
    */

    /*
    for(i = 0; i < m; ++i)
    {
        scanf("%d %d", &temp1, &temp2);

        if(temp1 == -1 || temp2 == -1)
        {
            fprintf(stderr, "scanf failed\n");
            exit(1);
        }

        adj[temp1][temp2] = 1;
        adj[temp2][temp1] = 1;

        degree[temp1] += 1;
        degree[temp2] += 1;

        if(temp1 == temp2)
            degree[temp1] -= 1;
    }
    */
    for(i = 0; i < m; ++i)
    {
        temp1 = lhs_buf[i];
        temp2 = rhs_buf[i];

        adj[temp1][temp2] = 1;
        adj[temp2][temp1] = 1;
    }

    /*
    for(i = 0; i < MAXSIZE; ++i)
    {
        for(j = adj_v[i].begin(); j != adj_v[i].end(); ++j)
        {
            printf("(%d, %d) = %d\n", i, 
        }
    }
    */

    for(i = 0; i < n; ++i)
    {
        if(DEBUG) printf("********%d-th dp********\n", i);
        dp_res[i] = dp(i, visited);
        if(DEBUG) printf("********%d-th recursion end, result : %d********\n", i, dp_res[i]);
        if(dp_res[i] > max_len)
            max_len = dp_res[i];
    }

    visited = (char *)malloc(n * sizeof(char));
    lhs_buf = (int *)malloc(m * sizeof(int));
    rhs_buf = (int *)malloc(m * sizeof(int));

    for(i = 0; i < n; ++i)
    {
        if(adj[i] == NULL)
            continue;

        free(adj[i]);
    }
    free(adj);

    printf("%d\n", max_len);
    return 0;
}
