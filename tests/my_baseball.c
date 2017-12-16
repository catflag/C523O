#include <stdio.h>
#include <stdlib.h>
#include <time.h>



int main()
{
    int X, Y, Z;
    int gX, gY, gZ;
    int strike;
    int ball;
//  int isClear = 0;
    int opt = 2;
    
    puts("Give me 1 if you want to play, otherwise, exit");
    scanf("%d", &opt);
        
    if (opt != 1)
    {
        puts("bye.");
        return 0;
    }
    srand(time(NULL));
    X = rand() % 9 + 1;
    Y = rand() % 9 + 1;
    while(X == Y)
        Y = rand() % 9 + 1;
    Z = rand() % 9 + 1;
    while(Z == X || Z == Y)
        Z = rand() % 9 + 1;

    while (1)
    {
        puts("COME ON! Guess three numbers");
        scanf("%d %d %d", &gX, &gY, &gZ);
        strike = (X == gX) + (Y == gY) + (Z == gZ);
        if (strike == 3){
            puts("Congratuation! YOU GOT IT!");
            printf("The answer was %d, %d, %d !\n", X, Y, Z);
            break;
        }
        ball = (X == gY) + (X == gZ) + (Y == gX) + (Y == gZ) + (Z == gX) + (Z == gY);
        printf("strike : %d, ball : %d\n", strike, ball);
    }
    return 0;
}
