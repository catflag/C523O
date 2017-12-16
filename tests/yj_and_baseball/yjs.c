#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char *argv[]) {
   int a, b, c;
   int x, y, z;
   int X, Y, Z;
   int A1, A2, A3;
   int Count, num=1;
   int Strike=0, Ball=0;
   printf("Input mode (1: Play, 2: Test, 0: Exit):");
   scanf("%d", &a);
   if (a==0){
      return 0;
   }
   if (a==1){
    srand(time(NULL));
      X=rand() %9+1;
      Y=rand() %9+1;
      Z=rand() %9+1;
      printf("Set difficulties (1: Easy, 2: Normal, 3: Hard): ");
      scanf("%d", &b);
      printf("Let's play baseball game!\n");
      if (b==1){
         for(Count=1; Count<=15; Count++){
            printf("Stage %d - Input 3 numbers (a b c):", num);
            scanf("%d %d %d", &A1, &A2, &A3);
            if (A1==X){
               Strike++;
            }
            else if (A1==Y||A1==Z){
               Ball++;
            }
            else{
               goto End;
            }
            if (A2==Y){
               Strike++;
            }
            else if (A2==X||A2==Z){
               Ball++;
            }
            else{
               goto End;
            }
            if (A3==Z){
                Strike++;
            }
            else if (A3==Z||A3==Y){
               Ball++;
            }
            else{
               goto End;
            }
            End:
            printf("Stage %d results: %d strike(s), %d ball(s)\n", num, Strike, Ball);
            if (Strike==3){
               printf("Congratuation!\n");
               break;
            }
            if (num>15){
               printf("You failed\n");
               break;
            }
            Strike=0;
            Ball=0;
            num++;
         }
      }
   }
      if (b==2){
            for(Count=1; Count<=10; Count++){
               printf("Stage %d - Input 3 numbers (a b c):", num);
               scanf("%d %d %d", &A1, &A2, &A3);
               if (A1==X){
               
                  Strike++;
               }
               else if (A1==Y||A1==Z){
                  
                  Ball++;
               }
               else{
                  
                  goto EnD;
               }
               if (A2==Y){
               
                  Strike++;
               }
               else if (A2==X||A2==Z){
                  
                  Ball++;
               }
               else{
                  
                  goto EnD;
               }
               if (A3==Z){
               
                  Strike++;
               }
               else if (A3==Z||A3==Y){
                  
                  Ball++;
               }
               else{
                  goto EnD;
               }
               EnD:
               printf("Stage %d results: %d strike(s), %d ball(s)\n", num, Strike, Ball);
               if (Strike==3){
                  printf("Congratuation!\n");
                  break;
               }
               if (num==10){
                  printf("You failed\n");
                  break;
               }
               Strike=0;
               Ball=0;
               num++;
            }
         }
      if (b==3){
            for(Count=1; Count<=5; Count++){
               printf("Stage %d - Input 3 numbers (a b c):", num);
               scanf("%d %d %d", &A1, &A2, &A3);
               if (A1==X){
               
                  Strike++;
               }
               else if (A1==Y||A1==Z){
                  
                  Ball++;
               }
               else{
                  
                  goto END;
               }
               if (A2==Y){
               
                  Strike++;
               }
               else if (A2==X||A2==Z){
                  
                  Ball++;
               }
               else{
                  
                  goto END;
               }
               if (A3==Z){
               
                  Strike++;
               }
               else if (A3==Z||A3==Y){
                  
                  Ball++;
               }
               else{
                  goto END;
               }
               END:
               printf("Stage %d results: %d strike(s), %d ball(s)\n", num, Strike, Ball);
               if (Strike==3){
                  printf("Congratuation!\n");
                  break;
               }
               if (num==5){
                  printf("You failed\n");
                  break;
               }
               Strike=0;
               Ball=0;
               num++;
            }
         }
   if (a==2){   
      srand(time(NULL));
      X=rand()%9+1;
      Y=rand()%9+1;
      Z=rand()%9+1;
      printf("You're in test mode.\n");
      printf("Numbers are %d %d %d\n", X, Y, Z);
      printf("Set difficulties (1: Easy, 2: Normal, 3: Hard): ");
      scanf("%d", &c);
      printf("Let's play baseball game!\n");
      if (c==1){
         for(Count=1; Count<=15; Count++){
            printf("Stage %d - Input 3 numbers (a b c):", num);
            scanf("%d %d %d", &A1, &A2, &A3);
               if (A1==X){
            
                  Strike++;
               }
               if (A1==Y||A1==Z){
                  
                  Ball++;
               }
               else{
               
                  goto clear;
               }
               if (A2==Y){
                  
                  Strike++;
               }
               if (A2==X||A2==Z){
                     
                  Ball++;
               }
               else{
                     
                  goto clear;
               }
               if (A3==Z){
                  
                  Strike++;
               }
               if (A3==Z||A3==Y){
                     
                  Ball++;
               }
               else{
                  goto clear;
               }
               clear:
               printf("Stage %d results: %d strike(s), %d ball(s)\n", num, Strike, Ball);
               if (Strike==3){
                  printf("Congratuation!\n");
                  break;
               }
               if (num==15){
               printf("You failed\n");
               break;
               }
               Strike=0;
               Ball=0;
               num++;
               }
            }
         }
      else{
          return 0;
      }   
   return 0;
}
