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

static int create_sem( int key,int el_number,int value);
static void set_sem(int semid,int number, int binary_val);
static void p(int semid,int number);
static void v(int semid, int number);

int* licznikptr;

int w1 = 0;//wykluczenie wzajemne procesow czytajacych w chwili rozpoczynania i konczenia korzystania z zasobu
int w2 = 1;//wykluczenie wzajemnie procesow piszacych w chwili rozpoczynania i konczenia korzystania z zasobu
int w3 = 2;//priorytet pisania nad czytaniem
int sp = 3;//wykluczenie wzajemne procesu piszacego wzgledem innych procesow
int sc = 4;//ochrona wejscia do sekcji krytycznej


int main(int argc , char * argv[])
{
	int m=strtoul(argv[1],NULL,10);
	if(errno == ERANGE)
	{
	  perror("[writer] strtoul error");
	  exit(EXIT_FAILURE);
	}
	
	srand(time(NULL));
	//Generuje klucz
	key_t klucz=ftok(".",'F');

	//Tworze nowy zbior semaforow o podanym kluczu
	int semID=create_sem(klucz,5,1);	
	
	
	key_t kluczLicznika=ftok(".",'P');
	int  licznikid=shmget(kluczLicznika, 4, IPC_CREAT|0666);
	if(licznikid == -1)
	{
		perror("[writer] shmget error");
		exit(EXIT_FAILURE);
	}
	//Dolaczam segment pamieci wspoldzielonej do procesu
	licznikptr=shmat(licznikid,0,0);//licznik pisarzy
    if((*licznikptr)==-1)
    {
		perror("[writer] shmat error");
		exit(EXIT_FAILURE);
	}

//pseudokod
	while(1)
	{		
	//czekaj na  pisarzy
	p(semID,w2);//wait
    (*licznikptr)=(*licznikptr)+1;//wpusc pisarza
    if((*licznikptr)==1)
    {//czekaj na mozliwosc wejscia do sekcji krytycznej
        p(semID,sc);//wait sc
    }
	//wznow  pisarzy
    v(semID,w2);//signal
	//czekaj az wejdzie jeden pisarz
    p(semID,sp);//wait
    //sekcja krytyczna
    FILE *input;
	//pisanie
	if((input=fopen("czytelnia", "w"))) //write
	{
        char sign = ('A' + (rand() % 25));
        printf("[Pisarz %d]\tZapisuje do czytelni znak %c\n",getpid(),sign);
        fputc(sign, input);
        fclose(input);
    }
	sleep(2);//zeby czytelnik cokolwiek wypisal
	//koniec sekcji krytycznej
	//wznow pisarza
    v(semID,sp);//signal
	//czekaj na pisarzy
    p(semID,w2);//wait
    (*licznikptr)=(*licznikptr)-1;
    if ((*licznikptr)==0)
    {
		//wejdz do sekcji krytycznej
        v(semID,sc);//signal
    }
	//wznow pisarzy
    v(semID,w2);//signal
}
}


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
                perror("[writer] semctl SETVAL erorr");
                exit(EXIT_FAILURE);
            }            
		}	
  		return semid;
    }
	perror("[writer]semget IPC_CREAT|IPC_EXECL error");
	exit(EXIT_FAILURE);
}


static void set_sem(int semid,int number, int binary_val)
{
    if (semctl(semid,number,SETVAL,binary_val)==-1)
    {
        perror("[writer]semctl SETVAL error");
        exit(EXIT_FAILURE);
    }

}

static void p(int semid,int number)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=number;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semid,&bufor_sem,1);
    if (zmien_sem==-1) 
    {	
    	//ten algorytm z projektu 4 
    if(errno==EINTR)
		{
			p(semid,number);
		}
		else
		{
			perror("[writer]semop  error");
			exit(EXIT_FAILURE);
		}
    }
}

static void v(int semid, int number)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=number;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semid,&bufor_sem,1);
  if (zmien_sem==-1) 
    {	
    	//ten algorytm z projektu 4 
    if(errno==EINTR)
		{
			p(semid,number);
		}
		else
		{
			perror("[writer]semop  error");
			exit(EXIT_FAILURE);
		}
    }
}
