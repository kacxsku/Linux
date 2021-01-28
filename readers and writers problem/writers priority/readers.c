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

static int create_sem( int key,int el_number,int value);
static void set_sem(int semid,int sem_num, int binary_val);
static void semafor_p(int semid,int sem_num);
static void semafor_v(int semid, int sem_num);

int w1 = 0;//wykluczenie wzajemne procesow czytajacych w chwili rozpoczynania i konczenia korzystania z zasobu
int w2 = 1;//wykluczenie wzajemnie procesow piszacych w chwili rozpoczynania i konczenia korzystania z zasobu
int w3 = 2;//priorytet pisania nad czytaniem
int sp = 3;//wykluczenie wzajemne procesu piszacego wzgledem innych procesow
int sc = 4;//ochrona wejscia do sekcji krytycznej

int *licznikptr2;

int main(int argc , char * argv[])
{
//pobieranie liczby m
int m=strtoul(argv[1],NULL,10);
if(errno == ERANGE)
	{
	  perror("[writer] strtoul error");
	  exit(EXIT_FAILURE);
	}

//Generuje klucz
key_t klucz=ftok(".",'F');

//Tworze nowy zbior semaforow o podanym kluczu
int semID=create_sem(klucz,5,1);


//Tworze licznik dzielony
key_t kluczLicznika=ftok(".",'L');
int  licznikid2=shmget(kluczLicznika, 4, IPC_CREAT|0666);
if(licznikid2 == -1)
{
	perror("[reader] shmget error");
	exit(EXIT_FAILURE);
}

//licznik
licznikptr2=shmat(licznikid2,0,0);//licznik czytelnikow
if(*licznikptr2==-1)
{
	perror("[reader] shmat error");
}
sleep(5);//zeby pisarz wszedl pierwszy
//pseudokod
while(1)
{
    //czekaj dopoki mozna pisac
    semafor_p(semID,w3);//wait
    //czekaj az bedzie mozna wejsc do sekcji krytycznej
    semafor_p(semID,sc);//wait
    //czekaj na czytelnikow
        //sekcja krytyczna
    semafor_p(semID,w1);//wait

    (*licznikptr2)=(*licznikptr2)+1;//zwiekszenie licznika
    if((*licznikptr2)==1)//jesli licznik ==1 czekaj na pisarza
    {//czekaj na pisarza
        semafor_p(semID,sp);//wait
    }
	if((*licznikptr2)>m)//sprawdzenie czy jest na tyle miejsc w czytelni
	{
		(*licznikptr2)=(*licznikptr2)-1;//wyrzuc jednego czytelnika
    //wznow procesy czytajace
		semafor_v(semID,w1);//signal
        //sekcja krytyczna
    	semafor_v(semID,sc);//signal
      //tera pisarz ma pierwszenstwo
    	semafor_v(semID,w3);//signal
		continue;
	}
  //wznow  czytelnikow
    semafor_v(semID,w1);//signal
    //wejdz sekcji krytycznej
    semafor_v(semID,sc);//signal
    //tera pisarz ma pierwszenstwo 
    semafor_v(semID,w3);//signal

    //czytanie;
    FILE *output;
    if((output=fopen("czytelnia", "r"))==NULL) 
	  {
        perror("[reader]fopen error");
        exit(-1);
    }
    char sign;
    sign = fgetc(output);
    printf("[Czytelnik %d]\tOdczytalem znak %c\n",getpid(),sign);
    fclose(output);
    sleep(1);
    //sekcji krytycznej
    //czekainie na zakonczenie czytania
    semafor_p(semID,w1);//wait
    (*licznikptr2)=(*licznikptr2)-1;//wyrzucenie czytelnika
    if((*licznikptr2)==0)//jesli licznik ==0 wpusc nowego pisarza
    {//wznowienie pisania 
        semafor_v(semID,sp);//signal
    }
    //wznow czytanie
    semafor_v(semID,w1);//signal
        //sekcja krytyczna
    }
}

static int create_sem( int key,int el_number,int value)
{
    int semafor=semget(key,el_number,IPC_CREAT|IPC_EXCL|0666);
   	if (errno==EEXIST) 
    {
		semafor=semget(key,el_number,IPC_CREAT|0666);    
  		return semafor;
  	}
    else
    {
		for(int i=0;i<el_number;i++)
		{
			set_sem(semafor,i,value);
		}	
  		return semafor;
    }
  perror("[reader]semget error");
	exit(EXIT_FAILURE);
}


static void set_sem(int semid,int number, int binary_val)
{
    if (semctl(semid,number,SETVAL,binary_val)==-1)
    {
        perror("[reader]semctl error");
        exit(EXIT_FAILURE);
    }

}

static void semafor_p(int semid,int number)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=number;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semid,&bufor_sem,1);
    if (zmien_sem==-1) 
    {	
    if(errno==EINTR)
		{
			semafor_p(semid,number);
		}
		else
		{
			perror("[reader]semop error");
			exit(EXIT_FAILURE);
		}
    }
}

static void semafor_v(int semid, int number)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=number;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semid,&bufor_sem,1);
  if (zmien_sem==-1) 
    {	
    if(errno==EINTR)
		{
			semafor_p(semid,number);
		}
		else
		{
			perror("[reader]semop  error");
			exit(EXIT_FAILURE);
		}
    }
}


