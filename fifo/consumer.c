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

/*TODO:
co sie dzieje w kodzie opisac
*/
//error handling
void error(char*string)
{
    perror(string);
    exit(EXIT_FAILURE);
}
FILE* system_process_limit;
FILE* actual_process_number; 
FILE* output;
int main(int argc,char* argv[])
{
    char file_name[30];
    //int chk=sprintf(file_name,"fwyj/wyj_%d",getpid());//zapis pliku do kat fwyj o nazwie wyj_pid
    //
    if(argc!=2)
        error("zla liczba argumentow wywolania");

    if((strchr(argv[1],'-'))!=NULL)
        error("liczba konsumentow: wystepuje znak '-'\n");
    
    unsigned long long consumer_number=strtoull(argv[1],NULL,10);//string,NULL,number system
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");

    //taking information about existing processes
    char existing_processes[256];//aktualne isniejace procesy
    //sprawdzenie liczby aktualnych procesow
    if((actual_process_number=popen("ps -u|grep kacper|wc -l","r"))==NULL)
        error("popen error");

    //przenies wszystko do tablicy 
    while((fgets(existing_processes,256,actual_process_number)));

    if(pclose(actual_process_number)==-1)//zamkniecie potoku
        error("pclose error");
	int fifo = open(NAZWA_FIFO, O_RDONLY);
	if(fifo == -1){
        printf("najpierw nalezy uruchomic producenta\n");
        error("open error");
    }
    sleep(4);
    unsigned long long existing_processes_number=strtoull(existing_processes,NULL,10);
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");
    printf("ex =%lld",existing_processes_number);
    //taking information about max user processes
    char proc_limit[256];
    //sprawdzenie limitu procesow
    if((system_process_limit=popen("bash -c 'ulimit -u'","r"))==NULL)//bash -c to  polecenia  odczytywane są z łańcucha.  Jeżeli po łańcuchu istnieją argumenty, 
        error("popen error");                                        //to  są  one przypisywane do argumentów pozycyjnych, poczynając od $0.

    //przenies wszystko do tablicy 
    while((fgets(proc_limit,256,system_process_limit)));

    if(pclose(system_process_limit)==-1)//zamkniecie potoku
        error("pclose error");
    //printf("limit procesow %s\n",proc_limit);
    unsigned long  proc_limit_number=strtoul(proc_limit,NULL,10);
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");
    //proc_limit_number=10000;
    //checking posibility of creating this number of procecesses
    long long processes_sum =existing_processes_number+consumer_number;
    if(processes_sum>proc_limit_number)//sprawdzenie czy liczba uruchamianych procesow przez program nie przekroczy max liczby procesow
        error("Limit procesow zostal przekroczony ");


    int stat;
    char sign;
    //opening fifo
        //access?????????????????????????????????????????????????????????????????????????????????????
    memset(&sign, '\0', sizeof(char));

    for(int i=0;i<consumer_number;i++)
    {
        switch(fork())
        {
            case -1:
                error("fork error");
            case 0:
                sleep(1);//czekaj az konsument skonczy 
                if(sprintf(file_name,"fwyj/wyj_%d",getpid())<0)//saving filename to array
                    error("sprintf error");
                FILE* output;//new output
                if((output=fopen(file_name,"w"))==NULL)
                    perror("file open error");
                while(read(fifo,(void *)&sign,sizeof(char))>0)//read data from fifo
                {
                    printf("proces %d pobral znak: %c\n",getpid(),sign);
                    if(fprintf(output,"%c",sign)<0)//save data into file
                        error("fprintf error");        
                }
                if(fclose(output)==EOF) //close output
                    error("fclose error");
                exit(0);
            default:
                break;
        }
        
    }
    close(fifo);
    /*if(close(fifo)==-1)
        error("close error");*/
    int status=-1;
    int  pid=0;

    for (int i=0;i<consumer_number;i++)
    {
        pid= wait(&status);
        if(status==-1)
        {
            error("wait error");
        }
        printf("proces : %d zakonczyl dzialanie. Kod powrotu :%d\n",pid,status);
    }
    if(unlink(NAZWA_FIFO)==-1)
        error("unlink error");
    return 0;
}