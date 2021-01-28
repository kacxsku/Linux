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

static int create_sem( int klucz,int liczba_elementow,int wartosc_semafora);
static void semafor_p(int semid,int numer_semafora);
static void semafor_p2(int semid,int numer_semafora1,int numer_semafora2);
static void semafor_v(int semid, int numer_semafora);
static void semafor_v2(int semid, int numer_semafora1, int numer_semafora2);
static void remove_sem(int semid);
void handler (int sig);

//struktura
struct philosopher {
    int name;//numer (nazwa) 
    int sem_numb;//numer semafora
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
    int zjadlem=0;
    for(int i=0; i<5;i++)
    {
        switch (fork())
        {
        case -1:
            perror("Fork error");
            break;
        case 0: //potomek
            while(1)//dikstra operacje jednoczesne
            {
            //printf("FILOZOF: %d mysli...\n",filozofowie[i].name);//myslenie
            sleep(1);//mysle
            //bierze oba widelce
            semafor_p2(semid,filozofowie[i].sem_numb,(filozofowie[i].sem_numb+1)%5);
            //czas na jedzenie 
            sleep(filozofowie[i].sem_numb*filozofowie[i].sem_numb); //jem
            //sleep(filozofowie[i].sem_numb);
            zjadlem++;
            printf("FILOZOF: %d zjadl %d\n",filozofowie[i].name,zjadlem);
            if(filozofowie[i].sem_numb==1||filozofowie[i].sem_numb==3)
            {
                usleep(rand()%5000);
            }
            //odklada oba widelce
            semafor_v2(semid,filozofowie[i].sem_numb,(filozofowie[i].sem_numb+1)%5);
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
		wait(NULL);
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

static void semafor_p2(int semid,int numer_semafora1,int numer_semafora2)
{
    int zmien_sem;
    struct sembuf bufor_sem[2];
    bufor_sem[0].sem_num=numer_semafora1;
    bufor_sem[0].sem_op=-1;
    bufor_sem[0].sem_flg=0;
    
    bufor_sem[1].sem_num=numer_semafora2;
    bufor_sem[1].sem_op=-1;
    bufor_sem[1].sem_flg=0;
    zmien_sem=semop(semid,bufor_sem,2);

   if(zmien_sem==-1) 
    {  
        if(errno==EINTR)
        {
		semafor_p2(semid,numer_semafora1,numer_semafora2);
		}
		else if(errno==EIDRM)
        {
		  printf("Zestaw semaforow zostal już usunięty, błąd EIDRM\n");
		}
		else
        {
		  perror("semop error p2");
		  exit(EXIT_FAILURE);
	  	}
    }
}

  static void semafor_v2(int semid, int numer_semafora1, int numer_semafora2)
  {
    int zmien_sem;
    struct sembuf bufor_sem[2];
    bufor_sem[0].sem_num=numer_semafora1;
    bufor_sem[0].sem_op=1;
    bufor_sem[0].sem_flg=0;
    
    bufor_sem[1].sem_num=numer_semafora2;
    bufor_sem[1].sem_op=1;
    bufor_sem[1].sem_flg=0;
    zmien_sem=semop(semid,bufor_sem,2);
    if (zmien_sem==-1) 
      {
	  	if(errno==EIDRM){
		printf("Zestaw semaforow zostal już usunięty, błąd EIDRM\n");
		}
		else{
		perror("semop error v2");
        exit(EXIT_FAILURE);
	   }
      }
  }


static void remove_sem(int semid)  
{
    int sem;
    sem=semctl(semid,0,IPC_RMID);
    if (sem==-1)
    {
       perror("semctl IPC_RMID error");
        exit(EXIT_FAILURE);
    }
}
