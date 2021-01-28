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
//defined global variables
#define MAX 8184
#define SERVER 1
#define CLIENT 2

//message structur
struct message
{
    long int mtype;//typ komunikatu w funkcji odbiorczej 
    long int pid;//adres zwrotny - typ komunikatu na ktore bedzie czekal dany klient
    char text[MAX];
};

key_t key;
int QueryID;

void createQuery();
void sigint_handling();
void receive_message(struct message*);
void send_message(struct message*);
void remove_message();

int main()
{
    struct message mess;
    if((key=ftok(".",'s'))==-1)
    {
        perror("[SERVER] ftok error");
        exit(EXIT_FAILURE);
    }
    createQuery();

    signal(SIGINT,sigint_handling);//singal SIGINT handling
    printf("^c konczy prace serwera i zamyka kolejke\n");
    
    while(1)
    {
        printf("[SERVER] oczekuje na komunikat\n");
        mess.mtype=SERVER;
        receive_message(&mess);
        mess.mtype=mess.pid;
        int mess_size=strlen(mess.text);
        for (int i = 0; i < mess_size; i++)
        {
            if(mess.text[i]>='a' && mess.text[i]<='z')
            {
                mess.text[i]= toupper(mess.text[i]);//change every small letter to upper
            }
        }
        mess.mtype=mess.pid;
        send_message(&mess);
    }
}

void createQuery()
{
    if((QueryID=msgget(key,IPC_CREAT | 0666))==-1)
    {
        perror("[Server] msgget(pipe create) error");
        exit(EXIT_FAILURE);
    }
    else printf("kolejka zostala utworzona! ID kolejki : %d\n",QueryID);
}

void sigint_handling(int signal)
{
    if(signal==SIGINT)//Znak przerwania
    {
        printf("[SERVER] koncze prace serwera\n");
        remove_message();
        exit(0);
    }
}

void receive_message(struct message *mess)
{
    //odbieram max+sizeof(long) bo nie znam rozmiaru komunikatu 
    if((msgrcv(QueryID,mess,MAX+sizeof(long),mess->mtype,0))==-1)
    {
        perror("[SERVER] msrcv error");
        remove_message();
        exit(EXIT_FAILURE);
    }
    else printf("[SERVER] odebrano: %s od %ld\n",mess->text,mess->pid);
}
void send_message(struct message *mess)
{
    //mozna wysylac strlen(send.text)+1+sizeof(long) 1-'\0' bo strlen podaje ilosc znakow strlen - dlugosc komunikatu, sizeof - bo oprocz komunikatu wysylam pid nadawcy
    //ale odbierac zawsze trzeba MAX+sizeof(long) bo nie znam dlugosci komunikatu
    if(msgsnd(QueryID,mess,MAX+sizeof(long),IPC_NOWAIT)==-1)//jesli kolejka sie przepelni to nie wysle wiadomosci i zakonczy sie bledem
    {
        perror("[SERVER] msgsnd error");
        remove_message();
        exit(EXIT_FAILURE);
    }
    else printf("[SERVER] wyslalem: %s do %ld\n",mess->text,mess->pid);
}

void remove_message()
{
    if(msgctl(QueryID,IPC_RMID,NULL)==-1)
    {
        perror("[SERVER] msgctl with IPC_RMID error");
        exit(EXIT_FAILURE);
    }
    else("kolejka zostala usunieta");
}
