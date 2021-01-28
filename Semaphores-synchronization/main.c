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
        perror("semget ERROR!");
        exit(1);
    }
    printf("semafor utworzony: %d\n",semaphore);
    return semaphore;
}
void process (char* name,char* path)
{
    switch (fork())
    {
    case -1:
        perror("fork ERROR!");
        exit(1);
        break;
    case 1:
    if(execl(path,name,NULL)==-1)
    {
        perror("execl ERROR!");
        exit(1);
    }
    default:
        break;
    }
}
FILE *file;
int main()
{
    file =fopen("program.txt","w");
    if(file==NULL)
    {
        perror("fopen ERROR!");
        exit(1);
    }
    int status;
    int ppid;
    int id;
    int Id=create(5);
    for (int i = 0; i <5 ; i++)
    {
        if(semctl(Id,i,SETVAL,0)==-1)
        {
            perror("semctl ERROR!");
            exit(1);
        }
    }
    printf("semafory zostaly ustawione\n");
   
       id = fork();
       if (id == 0) 
       {
       if(execl("1", "1", NULL) == -1)
       {
            perror("[2]fork ERROR!");
          	exit(1); 
	    }
        }
        id=fork();
		if (id == 0) 
        {
		if(execl("2", "2", NULL) == -1)
        {
		    perror("[2]fork ERROR!");
		    exit(1);
        }
        }
		id=fork();
		if (id == 0) 
        {
        if(execl("3", "3", NULL) == -1)
          {
            perror("[2]fork ERROR!");
  			exit(1);
		  }
        }


    for(int i=0;i<3;i++)
    {
        if(wait(&status)==-1)
        {
            perror("wait ERROR!");
            exit(1);
        }
    }
    if(semctl(Id,0,IPC_RMID)==-1)
    {
        perror("semctl ERROR!");
        exit(1); 
    }
    printf("semafor usuniety\n");
    fclose(file);
return 0;
}