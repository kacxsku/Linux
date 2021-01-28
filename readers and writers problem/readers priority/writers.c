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
#include<time.h>


static int create_sem( int klucz,int liczba_elementow,int wartosc_semafora);
static void semafor_p(int semid,int numer_semafora);
static void semafor_v(int semid, int numer_semafora);

int w = 0;
int sp = 1;


int main(int argc , char * argv[])
{
	srand(time(NULL));
	//generowanie klucza, taki sam jak w glownym i czytelniku
	key_t klucz=ftok(".",'F');

	//zbior 2 semaforow o wartosci 1
	int semID=create_sem(klucz,2,1);
	//pseudokod
	while(1)
	{
		//czekaj az bedzie mozna pisac
		semafor_p(semID,sp);
		//sekcja krytyczna
		FILE *input;
		if((input=fopen("czytelnia", "w"))) 
		{
        	char l = 0;
            l = ('A' + (rand() % 26));
            printf("[Pisarz %d]\tZapisuje do czytelni znak %c\n",getpid(),l);
            fputc(l, input);
        	fclose(input);
    	}
		sleep(1);
		//koniec sekcji krytycznej
    	semafor_v(semID,sp);//zakoncz pisac
	}

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

static void semafor_p(int semid,int numer_semafora)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=numer_semafora;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semid,&bufor_sem,1);
    if (zmien_sem==-1) 
    {	
        if(errno==EINTR)
		{
			semafor_p(semid,numer_semafora);
		}
		else if(errno==EIDRM)
		{
			printf("Zestaw semaforow zostal juz usuniety\n");
		}
		else
		{
			printf("Nie moglem zamknac semafora.\n");
			exit(EXIT_FAILURE);
		}
    }
}


static void semafor_v(int semid, int numer_semafora)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=numer_semafora;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semid,&bufor_sem,1);
    if (zmien_sem==-1) 
    {
	  	if(errno==EIDRM)
		{
			printf("Zestaw semaforow zostal juz usuniety\n");
		}
		else
		{
			printf("Nie moglem otworzyc semafora.\n");
        	exit(EXIT_FAILURE);
	   	}
	   
    }
}







