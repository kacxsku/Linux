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

//funkcje do semaforow
static int create_sem( int klucz,int liczba_elementow,int wartosc_semafora);
static void semafor_p(int semid,int numer_semafora);
static void semafor_v(int semid, int numer_semafora);

//numery semaforow
int w = 0;//wykluczenie wzajemne procesu czytajacego w chwili rozpoczynania i konczenia korzystania z zasobu
int sp = 1;//zapewnienie wykluczenia wzajemnego procesu piszacego wzgledem innych procesow
int *lc;
int  licznikid;//licznik 
FILE *output;//czytelnia

int main(int argc , char * argv[])
{
	
//pobieranie liczby m
int m=strtoul(argv[1],NULL,10);
if(errno == ERANGE)
{
	perror("[main] strtoul error");
	exit(EXIT_FAILURE);
}
//generowanie klucza semaforw, taki sam klucz co w glownym musi byc
key_t klucz=ftok(".",'F');

//zbior 2 semaforow o wartosci 1
int semID=create_sem(klucz,2,1);

//licznik dzielony o tym samym kluczu co w main.c
//shared_memory();
    key_t kluczLicznika=ftok(".",'L');
    //4 bajty bo rzutuje na inta potem
    licznikid=shmget(kluczLicznika, 0, IPC_CREAT|0666);//uzyskanie dostepu do pamieci dzielonej, size 0 bo segment istnieje
    if(licznikid == -1)
    {
	    perror("[reader]shmget IPC_CREAT error");
	    exit(EXIT_FAILURE);
    }

    //Dolaczam segment pamieci wspoldzielonej do procesu
    lc=shmat(licznikid,0,0);//
    if(*lc==-1)
    {
	    perror("[reader]shmat erorr");
    }

sleep(1);//zeby pisarz wszedl pierwszy
//pseudokod
while(1)
{
        //sekcja krytyczna
	//czekaj na proces czytajacy
    semafor_p(semID,w); //podniesienie semafora czytajacego
    (*lc)=(*lc)+1;  //zwiekszamy licznik o 1 bo wszedl nowy czytelnik
    //printf(":::::::::::::::::::::::::%d",*lc);
    //pisarz zawsze wejdzie pierwszy bo czytelnik czekac musi na to az bedzie cos zapisane, jak pisarz wyjdzie czytelnik bedzie mogl wejsc
	if((*lc)==1)
    {//czekaj na zapisanie czegos
        semafor_p(semID,sp); //jezeli jest to pierwwszy czytelnik to zmniejszamy semafor czytania
    }
	//sleep(1);
    
    //sprawdzanie miejsc w czytelni
	if((*lc)>m)
	{
         (*lc)=(*lc)-1;
         semafor_v(semID,w);//wznow czytanie
         continue;
    }
    //zacznij czytac
	semafor_v(semID,w); //czytelnik wchodzi do sekcji krytycznej, podnosimy semafor czytania
        //sekcja krytyczna

    if((output=fopen("czytelnia", "r"))==NULL) 
	{
        perror("[reader] fopen error");
        exit(-1);
    }
    char znak;
    znak = fgetc(output);
    printf("[Czytelnik %d]\tOdczytalem znak %c\n",getpid(),znak);
    fclose(output);
    sleep(1);
    //sekcji krytycznej

    //czekaj na proces czytajacy
	semafor_p(semID,w); 
    (*lc)=(*lc)-1; //obnizamy licznik no bo ten juz wyszedl
    if((*lc)==0) //jezeli to pierwszy czytelnik byl to podnosimy semafor dla pisarza
    {
        semafor_v(semID,sp);//wznow pisanie
    }
    //wznow czytanie
    semafor_v(semID,w);
    //sekcja krytyczna
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
			printf("[reader]Zestaw semaforow zostal juz usuniety\n");
		}
		else
		{
			printf("[reader]Nie moglem zamknac semafora.\n");
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
			printf("[reader]Zestaw semaforow zostal juz usuniety\n");
		}
		else
		{
			printf("[reader]Nie moglem otworzyc semafora.\n");
        	exit(EXIT_FAILURE);
	   	}
    }
}

