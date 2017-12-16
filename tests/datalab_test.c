#include <stdio.h>
#include <string.h>
#include <stdlib.h>



/*
 * replaceByte(x,n,c) - Replace byte n in x with c
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: replaceByte(0x12345678,1,0xab) = 0x1234ab78
 *   You can assume 0 <= n <= 3 and 0 <= c <= 255
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int test_repByte(int x, int n, int c)
{
	int shift_num = n << 3;
	int masking = ~(0xFF << shift_num); // ~ has more priority than <<.
	return (x & masking)|(c << shift_num);
}



int test_bang(int x)
{
    return ((x|(~x+1))>>31)+1;
}

int main()
{
    char testfunc[20];
    char input[20];
    int len;
    while(1)
    {
        printf("Give me what you want to test. If you want to finish, give me 'x'\n");
        printf("Current options are as follows\n");
        printf("bang, replace\n");
        len = read(0, testfunc, 20);
        testfunc[len-1] = '\0'; // or just 0;
        if(!strcmp("x", testfunc))
            break;
        if(!strcmp("bang", testfunc))
        {
            puts("argument : ");
            read(0, input, 20);
            printf("!%d = %d\n", atoi(input), test_bang(atoi(input)));        
        }
        else if(!strcmp("replace", testfunc))
        {
            int inputs[3];
            for(int i = 0; i < 3; ++i)
            {
                puts("argument : ");
                len = read(0,input,20);
                input[len-1] = 0;
                inputs[i] = atoi(input);
            }
            printf("rep(0x%x, %d, 0x%x) = %x\n", inputs[0], inputs[1], inputs[2], test_repByte(inputs[0],inputs[1], inputs[2]));
        }
    }
    return 0;
}
