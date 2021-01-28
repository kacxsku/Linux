#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
//defined global variables
#define MAX 8184//max size of text array
#define SERVER 1 //mtype of server
#define CLIENT 2//mtype of client
int sendcheck=0;
int grcv=0;
struct message
{
    long int mtype;//typ komunikatu w funkcji odbiorczej
    long int pid;//adres zwrotny - typ komunikatu na ktore bedzie czekal dany klient
    char text[MAX];//message text
};
//global structures
struct msqid_ds check;//odpowiada za kazda kolejke kommunikatow
struct message receive;
struct message send;
//global variables
key_t key;
int QueryID;
//functions
void createQuery();
void *send_message();
void *receive_message();

int main()
{
    pthread_t send_thread;
    pthread_t receive_thread;
    //creating key for msget function
    if((key=ftok(".",'s'))==-1)
    {
        perror("[SERVER] ftok error");
        exit(EXIT_FAILURE);
    }

    createQuery();
    //creating send pthread
    if(pthread_create(&send_thread,NULL,send_message,NULL)==-1)
    {
        perror("[CLIENT] pthread_create error");
        exit(EXIT_FAILURE);
    }
    else printf("[CLIENT] watek wysylajacy wiadomosc zostal utworzony\n");

    //creating receive thread
    if(pthread_create(&receive_thread,NULL,receive_message,NULL)==-1)
    {
        perror("[CLIENT] pthread_create error");
        exit(EXIT_FAILURE);
    }
    else printf("[CLIENT] watek odbierajacy wiadomosc zostal utworzony\n");
    //joining send thread
    if(pthread_join(send_thread,NULL)==-1)
    {
        perror("[CLIENT] pthread_join (send_thread) error");
        exit(EXIT_FAILURE);
    }
    else printf("[CLIENT] watek wysylajacy wiadomosc zostal dolaczony\n");

    //joining receive thread
    if(pthread_join(receive_thread,NULL)==-1)//join czeka az watek sie zakonczy
    {
        perror("[CLIENT] pthread_join (receive thread) error");
        exit(EXIT_FAILURE);
    }
    else printf("[CLIENT] watek odbierajacy wiadomosc zostal dolaczony\n");
    
    //detachnig send thread
    if(pthread_detach(send_thread)==-1)
    {
        perror("[CLIENT] pthread_detach(send_thread) error");
    }
    else printf("watek wysylajacy zostal odlaczony\n");
    //detachning receive thread
    if(pthread_detach(receive_thread)==-1)
    {
        perror("[CLIENT] pthread_detach(receive_thread) error");
    }
    else printf("watek odbierajacy zostal odlaczony\n");
}


void createQuery()
{
    if((QueryID=msgget(key,IPC_CREAT | 0666))==-1)
    {
        perror("[CLIENT] msgget(pipe create) error");
        exit(EXIT_FAILURE);
    }
    else printf("kolejka zostala utworzona! ID kolejki : %d\n",QueryID);
}


void *send_message()
{
        int i;
      while(1)
        {
            send.mtype=SERVER;
            send.pid=getpid();
            printf("[CLIENT] Podaj tresc komunikatu: \n");
        
            for (i = 0; i < MAX-1; i++)//dzielenie wiadomosci jesli jest za duza 
            {
                send.text[i]=getchar();//getchar pobiera pojedyncze znaki
                if((send.text[i]=='\n'))//jesli enter to stop
                {
                    break;
                }
            }

            send.text[i]='\0';//ostatni znak bez tego w printf sa problemy
            printf("[CLIENT] Wysylam: %s do %ld \n",send.text, send.mtype);

            //printf("[CLIENT] probuje wyslac wiadomosc: %s",send.text);
             //mozna wysylac strlen(send.text)+1+sizeof(long) 1-'\0' bo strlen podaje ilosc znakow strlen - dlugosc komunikatu, sizeof - bo oprocz komunikatu wysylam pid nadawcy
             //ale odbierac zawsze trzeba MAX+sizeof(long) bo nie znam dlugosci komunikatu
            if(msgsnd(QueryID,(struct message *)&send,MAX+sizeof(long),IPC_NOWAIT) == -1)               //przepelnienie kolejki
            {                                
                sendcheck=1;                                                                            //jesli kolejka sie przepelnia to nie wysle wiadomosci
                perror("[CLIENT] msgsnd error");                                                        //i zakonczy watek wysylajacy
                pthread_exit((void *)0);
            }
            else printf("[CLIENT] Wyslalem: %s do %ld \n",send.text, send.mtype);
            grcv++;
            sendcheck=0;
        }
}

void *receive_message()
{
         while(1)
        {
            

            //sprawdzenie czy kolejka jest pusta(dostep do mojej kolejki)
            if(msgctl(QueryID,IPC_STAT,&check) == -1)//IPC_STAT ustawia dane w strukturze msgid_ds zeby otrzymywal wartosci zwiazane z moja kolejka
            {
                perror("msgctl(ICT_STAT) error");
                exit(EXIT_FAILURE);
            }
            else
            {       //__msg_cbytes= Current number of bytes in queue
                if(check.__msg_cbytes==0 && sendcheck==1)//jesli kolejka jest pusta i watek send sie zakonczyl to watek receive nie jest potrzebny
                {
                    printf("[CLIENT] kolejka jest pusta i watek wysylajacy jest odlaczony wiec ja tez sie odlacze\n");
                    pthread_exit((void *)0);
                }
                else if (grcv!=0)
                {
                    receive.mtype = getpid();//przyjmuje wiadomosci tylko do siebie bo pid jest unikatowy dla kazdego procesu
                    // zawsze trzeba MAX+sizeof(long) bo nie znam dlugosci komunikatu
                    if(msgrcv(QueryID,(struct message *)&receive, MAX+sizeof(long), receive.mtype,0) == -1)//+sizeof(long)==rozmiar adresu odbiorcy
                    {
                        perror("[CLIENT] msrcv error");
                        exit(EXIT_FAILURE);
                    }
                    else printf("[CLIENT] Odebrano: %s od %ld\n", receive.text, receive.pid);
                }
            }


    
        }
}

