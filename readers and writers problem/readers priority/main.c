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
static int create_sem( int klucz,int liczba_elementow,int wartosc_semafora);

int* licznikptr;//
int  licznikid;//licznik 

//limity
FILE *actual_process, *process_limit;


int main(int argc, char *argv[]) 
{
		//niechce mi sie ciagle kompilowac
    /*system("gcc -o writers writers.c");

    system("gcc -o readers readers.c");*/
	char actual_proc_arr[256];
	char limit_arr[256];

	if(argc != 4)
	{
		perror("Zla liczba argumentow!\n");
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
			perror("Przepelnienie strtoul!");
			exit(EXIT_FAILURE);
		}	
	
	unsigned long liczba_c = strtoul(argv[2], NULL, 0);//czytelnicy
	if(errno == ERANGE)
		{
			perror("Przepelnienie strtoul!");
			exit(EXIT_FAILURE);
		}	
	
	unsigned long liczba_m = strtoul(argv[3], NULL, 0);//liczba miejsc
	if(errno == ERANGE)
	{
			perror("Przepelnienie strtoul!");
			exit(EXIT_FAILURE);
	}

	//kontrola procesow w systemie
	long long proc_sum=liczba_proc + liczba_p + liczba_c;
	if(limit < proc_sum)
	{
		perror("Limit procesow zostal przekroczony!\n");
		exit(EXIT_FAILURE);
	}
	
	//tworzenie licznika zeby go w handlerze usunac(=> usuniety w kazdym programie), w czytelniku sie dolaczamy do pamiecie dzielonej
	key_t kluczLicznika=ftok(".",'L');
	//4 bajty bo rzutuje na inta potem
	licznikid=shmget(kluczLicznika, 4, IPC_CREAT|0666);
	if(licznikid == -1)
	{
		perror("[main] shmget error");
		exit(EXIT_FAILURE);
	}

	//Dolaczam segment pamieci wspoldzielonej do procesu
	licznikptr=shmat(licznikid,0,0);
	if(*licznikptr==-1)
	{
		perror("[main] shmat error");
	}
		
	//tworzenie procesow pisarzy
    for(int i=0; i<liczba_p; i++)
    {
        switch(fork())
        {
            case -1:
                perror("[main] fork error");
                exit(EXIT_FAILURE);
            case 0:
            	

                if(execl("./writers", "writers", NULL)==-1)
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
                exit(-1);
            case 0:
				//do czytelnika przekazujemy liczbe m bo jest nam potrzebna w algorytmie
                if(execl("./readers" , "readers",argv[3],NULL)==-1)
                {
                    perror("[main] execl error");
                    exit(EXIT_FAILURE);
                }
        }
    }
 	
//przechwytywanie sygnalow
signal(SIGINT, handler);
 
 	//czekamy na zakonczenie procesow potomnych
for(int i=0;i<liczba_p+liczba_c;i++)
    {
		wait(NULL);
	}

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
   int sem=semctl(semid,0,IPC_RMID);
    if (sem==-1)
    {
       	perror("[main] semctl IPC_RMID error");
        exit(EXIT_FAILURE);
    }
  //usuwam segment pamieci dzielonej licznika ktora przydzielismy wyzej
  if(shmdt(licznikptr)==-1)
  {
      perror("[main] shmdt error");
  }
  if(shmctl(licznikid,IPC_RMID,0)==-1)
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
                perror("[reader] semctl SETVAL erorr");
                exit(EXIT_FAILURE);
            }            
		}	
  		return semid;
    }
	perror("[reader]semget IPC_CREAT|IPC_EXECL error");
	exit(EXIT_FAILURE);
}


