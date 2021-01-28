#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define NAZWA_FIFO "./moje_fifo"


//TODO:
//SPRAWDZENIE CZY FIFO ISTNIEJE
//handling errors
void error(char*string)
{
    perror(string);
    exit(EXIT_FAILURE);
}
FILE* system_process_limit;
FILE* actual_process_number; 
int fifo;
int main(int argc,char* argv[])
{
    char file_name[30];

    if(argc !=3)//nazwa l.producentow l.konsumentow l.znakow
        error("zla liczba argumentow wywolania\n");
    
     if((strchr(argv[1],'-'))!=NULL)//znak
        error("liczba producentow: wystepuje znak '-'\n");
    
    unsigned long long producer_number=strtoull(argv[1],NULL,10);//string,NULL,number system
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");

    if((strchr(argv[2],'-'))!=NULL)
        error("liczba konsumentow: wystepuje znak '-'\n");
    
    unsigned long  char_number=strtoul(argv[2],NULL,10);
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");
        
    //sprawdzenie czy nie podano za duzo znakow
    if(char_number>=PIPE_BUF)//4096
        error("za duzo znakow");
    //check actual processes number

    if(access(NAZWA_FIFO,F_OK)==-1)//
    {   //otwarcie lacza
        int check_mkfifo=mkfifo(NAZWA_FIFO,0600);//minimalne prawa 
        if(check_mkfifo!=0)
            error("mkfifo error");
    }
    fifo=open(NAZWA_FIFO,O_WRONLY);//otworzenie tylko do zapisu
    if(fifo==-1)
        error("open error");

    char existing_processes[256];//aktualne isniejace procesy
    //sprawdzenie liczby aktualnych procesow
    if((actual_process_number=popen("ps -ux|wc -l","r"))==NULL)
        error("popen error");

    //przenies wszystko do tablicy 
    while((fgets(existing_processes,256,actual_process_number)));

    if(pclose(actual_process_number)==-1)//zamkniecie potoku
        error("pclose error");

    unsigned long long existing_processes_number=strtoull(existing_processes,NULL,10);
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");

    //check number of max user processes
    char proc_limit[256];
    //sprawdzenie limitu procesow
    if((system_process_limit=popen("bash -c 'ulimit -u'","r"))==NULL)//bash -c to  polecenia  odczytywane są z łańcucha.  Jeżeli po łańcuchu istnieją argumenty, 
        error("popen error");                                        //to  są  one przypisywane do argumentów pozycyjnych, poczynając od $0.

    //przenies wszystko do tablicy 
    while((fgets(proc_limit,256,system_process_limit)));

    if(pclose(system_process_limit)==-1)//zamkniecie potoku
        error("pclose error");

    unsigned long  proc_limit_number=strtoul(proc_limit,NULL,10);
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");
    //sprawdzenie czy nie za duzo procesow chcemy wywolac
    long long processes_sum =existing_processes_number+producer_number;
    if(processes_sum>proc_limit_number)//sprawdzenie czy liczba uruchamianych procesow przez program nie przekroczy max liczby procesow
        error("Limit procesow zostal przekroczony ");
    
//fifo
   

    char sign;
    unsigned long counter;
    for(int i=0;i<producer_number;i++)
    {
        switch(fork())
        {
            case -1:
                error("fork error");
            case 0:
                sleep(3);
                if(sprintf(file_name,"fwe/we_%d",getpid())<0)//zapis nazwy pliku do zapisu
                    error("sprintf error");
                FILE* input;
                if((input=fopen(file_name,"w"))==NULL)
                    perror("file open error");
                
                for(int i=0;i<char_number;i++)
                    {
                        sign=(rand()%26+'a');
                        printf("producent %d wylosowal znak :%c\n",getpid(),sign);
                        if(fprintf(input,"%c",sign)<0)//zapis znakow do pliku
                            error("fprintf error");        
                            //zapis do fifo
                        if((write(fifo,&sign,sizeof(char)))==-1)//zapis znakow do potoku
                            error("write error");
                    }
                if(fclose(input)==EOF)
                    error("fclose error");
                exit(0);
            default:
                break;
        }//oczekiwanie  na zakonczenie potomka
       
    }
     if(close(fifo)==-1)
        error("close error");

    int status=-1;
    int  pid=0;

    for (int i=0;i<producer_number;i++)
    {
        pid= wait(&status);
        if(status==-1)
        {
            error("wait error");
        }
        printf("proces : %d zakonczyl dzialanie. Kod powrotu :%d\n",pid,status);
    }

    return 0;
}