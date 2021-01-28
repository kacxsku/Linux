#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/resource.h>


void handler (int sig);
static int create_sem( int key,int el_number,int value);
static void set_sem(int semid,int sem_number, int binary_val);

//liczniki
int* licznikptr;
int  licznikid;

int* licznikptr2;
int  licznikid2;

//limity
FILE *actual_process, *process_limit;

int main(int argc, char *argv[]) 
{
	//niechce mi sie ciagle kompilowac
   /* system("gcc -o writers writers.c");

    system("gcc -o readers readers.c");*/
	char actual_proc_arr[256];
	char limit_arr[256];
 	if(argc != 4)
	{
		perror("[main] Zla liczba argumentow!\n");
		exit(EXIT_FAILURE);	
	}

    //sprawdzenie liczby dzialajacych procesow
	actual_process = popen("ps ux | grep sostudent63 | wc -l", "r"); 
	while(fgets(actual_proc_arr, 256, actual_process));
	pclose(actual_process);
	unsigned long liczba_proc = strtoul(actual_proc_arr, NULL, 0);
	if(errno == ERANGE)
	{
		perror("[main] strtoul error");
		exit(EXIT_FAILURE);
	}
	
	//sprawdzenie limitu procesow w systemie
	process_limit = popen("ulimit -u", "r");
	while(fgets(limit_arr, 256, process_limit));
	pclose(process_limit);
	unsigned long limit = strtoul(limit_arr, NULL, 0);
	if(errno == ERANGE)
	{
		perror("[main] strtoul error");
		exit(EXIT_FAILURE);
	}

	unsigned long liczba_p = strtoul(argv[1], NULL, 0);//pisarze
	if(errno == ERANGE)
	{
		perror("[main] strtoul error");
		exit(EXIT_FAILURE);
	}

	unsigned long liczba_c = strtoul(argv[2], NULL, 0);//czytelnicy
	if(errno == ERANGE)
	{
		perror("[main] strtoul error");
		exit(EXIT_FAILURE);
	}
	
	unsigned long liczba_m = strtoul(argv[3], NULL, 0);//liczba miejsc
	if(errno == ERANGE)
	{
		perror("[main] strtoul error");
		exit(EXIT_FAILURE);
	}

	//kontrola procesow w systemie
	long long proc_sum=liczba_proc + liczba_p + liczba_c;
	if(limit < proc_sum)
	{
		perror("[main]Limit procesow zostal przekroczony");
		exit(EXIT_FAILURE);
	}

	//Tworze pamiec dzielona
	key_t kluczLicznika=ftok(".",'P');
	licznikid=shmget(kluczLicznika, 4, IPC_CREAT|0666);//4 bajty bo rzutuje na int
	if(licznikid == -1)
	{
		perror("[main] shmget IPC_CREAT error");
		exit(EXIT_FAILURE);
	}

	//licznik dla pisarza
	licznikptr=shmat(licznikid,0,0);
	if(*licznikptr==-1)
	{
		perror("[main] shmat error");
	}
	
	//Tworze pamiec dzielona
	key_t kluczLicznika2=ftok(".",'L');
	licznikid2=shmget(kluczLicznika2, 4, IPC_CREAT|0666);//4 bajty bo rzutuje na int
	if(licznikid2 == -1)
	{
		perror("[main] shmget IPC_CREAT error");
		exit(EXIT_FAILURE);
	}

	//licznik dla czytelnika
	
	licznikptr2=shmat(licznikid,0,0);
	if(*licznikptr2==-1)
	{
		perror("[main] shmat error");
	}
	//tworzenie procesow pisarzy
	//do pisarz i czytelnikow musze przekazac liczbe miejsc w czytelni bo tym razem pisarze nie wchodza i wychodza odrazu 
	//tylko moga siedziec
    for(int i=0; i<liczba_p; i++)
    {
        switch(fork())
        {
            case -1:
                perror("[main] fork error");
                exit(EXIT_FAILURE);
            case 0:
                if(execl("./writers", "writers",argv[3],NULL)==-1)
                {
                    perror("[main] execl error");
                    exit(EXIT_FAILURE);
                }
        }
    }
//tworzenie procesow czytelnika
    for(int i=0; i<liczba_c; i++)
    {
       
        switch(fork())
        {
            case -1:
                perror("[main] fork error");
                exit(EXIT_FAILURE);
            case 0:
                if(execl("./readers" , "readers",argv[3],NULL)==-1)
                {
                    perror("[main] execl error");
                    exit(EXIT_FAILURE);
                }
        }
    }
 	
 	signal(SIGINT, handler);
 
 	for(int i=0;i<liczba_p+liczba_c;i++)
        wait(NULL);

 return 0;	
}


void handler (int sig)
{
	printf("SIGINT!!!\n usuwam semafory i pamiec dzielona\n");
 //tworze semafory o podanym kluczu
 //tak naprawde to sie dolaczam do tych z programow potomnych
 //a nastepnie je usuwam
  	key_t klucz=ftok(".",'F');
  	int semid=create_sem(klucz,2,1);
   	int sem=semctl(semid,0,IPC_RMID);//usuniecie semafora
    if (sem==-1)
    {
       	perror("[main] semctl IPC_RMID error");
        exit(EXIT_FAILURE);
    }
  	if(shmdt(licznikptr)==-1)
  	{
    	perror("[main] shmdt error");
  	}
  	if(shmctl(licznikid,IPC_RMID,0)==-1)
  	{
    	perror("[main] shmctl IPC_RMID error");
  	}
  	if(shmdt(licznikptr2)==-1)
  	{
    	perror("[main] shmdt error");
  	}
  	if(shmctl(licznikid2,IPC_RMID,0)==-1)
  	{
    	perror("[main] shmctl IPC_RMID error");
  	}
  	exit(EXIT_FAILURE);
}

//tworzenie i ustawienie semaforow o ile jeszcze nie sa ustawione
static int create_sem( int klucz,int liczba_elementow,int wartosc_semafora)
{
    int semid=semget(klucz,liczba_elementow,IPC_CREAT|IPC_EXCL|0666);
   	if (errno==EEXIST) 
    {
		semid=semget(klucz,liczba_elementow,IPC_CREAT|0666);    
  		return semid;
  	}
    else
    {
		for(int i=0;i<liczba_elementow;i++)
		{
            int ustaw_sem;
            ustaw_sem=semctl(semid,i,SETVAL,wartosc_semafora);
            if (ustaw_sem==-1)
            {
                perror("[main] semctl SETVAL erorr");
                exit(EXIT_FAILURE);
            }            
		}	
  		return semid;
    }
	perror("[main]semget IPC_CREAT|IPC_EXECL error");
	exit(EXIT_FAILURE);
}

