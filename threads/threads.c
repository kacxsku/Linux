#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// kompilacja: gcc projekt_2.c -lpthread
//wymiary 
#define WIDTH  10
#define HEIGHT  2
//tabela
int tab[HEIGHT][WIDTH];
//sumy
int sum1 = 0;
int sum2 = 0;

void* watek_1()
{
    printf("TID: %ld",pthread_self());
    for(int i=0;i<WIDTH;i++)
    {
        sum1+=tab[0][i];
    }
    printf("suma elementow pierwszego wiersza tabeli (watek 1) = %d\n",sum1);
    pthread_exit((void *)0);
}

void* watek_2()
{
    for(int i=0;i<WIDTH;i++)
    {
        sum2+=tab[1][i];
    }
    printf("suma elementow drugiego wiersza tabeli (watek 2) = %d\n",sum2);
    pthread_exit((void *)0);
}


int main()
{
pthread_t tid1,tid2;

//wypelnienie liczbami losowymi %10
srand(time(NULL));
for(int i=0;i<HEIGHT;i++)
{
    for(int j=0;j<WIDTH;j++)
    {
        tab[i][j]=rand()%10;
    }
}
//wypisz macierz
printf("macierz wypełniona liczbami losowymi od 0 d o 10\n");
for(int i=0;i<HEIGHT;i++)
{
    for(int j=0;j<WIDTH;j++)
    {
        printf("%d ",tab[i][j]);
    }
    printf("\n");
}

printf("[system] wykonuje pthread_create \n");
//stworz watki poboczne 
if(pthread_create(&tid1,NULL,watek_1,NULL)==-1)
{
    perror("(watek 1) pthread_create ERROR!");
    exit(1);
}
if(pthread_create(&tid2,NULL,watek_2,NULL)==-1)
{
    perror("(watek 2) pthread_create  ERROR!");
    exit(1);
}

printf("[program] jestem program glowny\n");

printf("[program] czekam na przylaczenie watkow\n");
//przylacz watki
printf("[system] przylaczam watek 1\n");
if(pthread_join(tid1, NULL)==-1)
{
    perror("(watek 1) pthread_join  ERROR!");
    exit(1);
}
printf("[system] przylaczam watek 2\n");
if(pthread_join(tid2, NULL)==-1)
{
    perror("(watek 2) pthread_join  ERROR!");
    exit(1);
}
printf("Suma calkowita z sum cząstkowych = %d\n",sum1+sum2);


//odlaczenie watkow
printf("[system] odlaczam watek 1\n");
if(pthread_detach(tid1)==-1)
{
    perror("watek1 pthread_detach ERROR!");
}
printf("[system] odlaczam watek 2\n");
if(pthread_detach(tid2)==-1)
{
    perror("watek1 pthread_detach ERROR!");
}

printf("[program] koncze dzialanie\n");
exit(0);

}