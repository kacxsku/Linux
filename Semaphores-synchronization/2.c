#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
int create(int number)
{
    int key =11;
    int semaphore=semget(key,number,IPC_CREAT|0666);
    if ( semaphore==-1)
    {
        perror("[2] semget ERROR!");
        exit(1);
    }
    printf("[2] semafor utworzony: %d\n",semaphore);
    return semaphore;
}

int p(int Id,int sem_number)
{
    struct sembuf bufor_sem;
    bufor_sem.sem_num=sem_number;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=SEM_UNDO;
    if(semop(Id,&bufor_sem,1))
    {
        perror("[2]semop [zamkniecie semafora] ERROR!");
        exit(1);
    }
   printf("[2] semafor zostal zamkniety\n");

}
int v(int Id,int sem_number)
{
    struct sembuf bufor_sem;
    bufor_sem.sem_num=sem_number;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=SEM_UNDO;
    if(semop(Id,&bufor_sem,1))
    {
        perror("[2] semop [otwarcie semafora] ERROR!");
        exit(1);
    }
   printf("[2] semafor zostal otwarty\n");

}
FILE *file;
int main()
{
    printf("p222\n");
    file =fopen("program.txt","a");
    if(file==NULL)
    {
        perror("[2] fopen ERROR!");
        exit(1);
    }

    int Id =create(5);
    pid_t pid= getpid();
    fprintf(file,"Sekcja 21, PID procesu =%d\n",pid);
    fflush(file);
    sleep(1);
    v(Id,0);
    p(Id,1); 

    fprintf(file,"Sekcja 22, PID procesu =%d\n",pid);
    fflush(file);
    sleep(1);
    v(Id,2);
    p(Id,3);

    fprintf(file,"Sekcja 23, PID procesu =%d\n",pid);
    fflush(file);
    sleep(1);
    v(Id,4);
    fclose(file);
    exit(0);
}