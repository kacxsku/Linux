#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include<sys/wait.h>

static int  create_sem( int klucz,int liczba_elementow,int wartosc_semafora);
static void semafor_p(int semid,int numer_semafora);
static void semafor_v(int semid, int numer_semafora);
static void remove_sem(int semid);
void handler (int sig);

//struktura
struct philosopher {
    int name;
    int sem_numb;
};

int semid;



int main( int argc ,char * argv[])
{
    //Tworze klucz
    key_t klucz=ftok(".",'K');
    //Tworze widelce
    semid=create_sem(klucz,5,1);
    //Tworze tablice imion
    struct philosopher filozofowie[5];
    for(int i=0;i<5;i++)
    {
        filozofowie[i].name=i;
        filozofowie[i].sem_numb=i;
    }
//5 procesow potomych bo 5 filozofow
    for(int i=0; i<5;i++)
    {
        switch (fork())
        {
        case -1:
            perror("Fork error");
            break;
        case 0: //potomek
            while(1)
            {
                printf("FILOZOF: %d mysli\n",filozofowie[i].name);//myslenie
                //sleep(2);
                //opusc semafor [i]==wez widelec i lewy
                semafor_p(semid,filozofowie[i].sem_numb);//blokada kiedy kazdy podniesie lewy widelec
                //wez widelec i+1%5 prawy
                semafor_p(semid,(filozofowie[i].sem_numb+1)%5);
                printf("FILOZOF: %d je\n",filozofowie[i].name);//jeddzenie
                semafor_v(semid,filozofowie[i].sem_numb);//odloz widelec i lewy
                semafor_v(semid,(filozofowie[i].sem_numb+1)%5);//odloz widelec i+1%5 prawy
            }
            break;
        default:
            break;
        }
    }
    signal(SIGINT,handler);
    //Czekam na te procesy
   for(int i=0;i<5;i++)
    {
		wait(NULL);//nie zwraca informacji o zakonczonych procesach
	}
    remove_sem(semid);
}

void handler (int sig)
{
    printf("\nSIGINT\nusuwam semafor");
    remove_sem(semid);
    exit(EXIT_SUCCESS);
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
                perror("semctl SETVAL erorr");
                exit(EXIT_FAILURE);
            }            
		}	
  		return semid;
    }
	perror("semget IPC_CREAT|IPC_EXECL error");
	exit(EXIT_FAILURE);
}



static void semafor_p(int semid,int numer_semafora)
  {
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=numer_semafora;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    //printf("numer sem =%d\n",numer_semafora);
    zmien_sem=semop(semid,&bufor_sem,1);
    if (zmien_sem==-1) 
      {
        if(errno==EINTR){
		semafor_p(semid,numer_semafora);
		}
		else if(errno==EIDRM){
		printf("Zestaw semaforow zostal już usunięty, błąd EIDRM\n");
		}
		else{
		perror("semop error");
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
	  	if(errno==EIDRM){
		printf("Zestaw semaforow zostal już usunięty, błąd EIDRM\n");
		}
		else{
		perror("semop error v");
        exit(EXIT_FAILURE);
        }
      }
  }

  
static void remove_sem(int semid)  
{
    if (semctl(semid,0,IPC_RMID)==-1)
    {
       perror("semctl IPC_RMID error");
        exit(EXIT_FAILURE);
    }
}
