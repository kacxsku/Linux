#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>
#include<string.h>

/*argv:
name producer consumer chars*/
FILE* system_process_limit;
FILE* actual_process_number; 
void error(char*string)
{
    perror(string);
    exit(EXIT_FAILURE);
}


///sprawdzic liczbe znakow w plikach
int main(int argc, char const *argv[])
{
    char tab[256];

    /*na projekt wywalic!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    //niechce mi sie ciagle kompilowac
    /*system("gcc -o producer producer.c");
    if(errno==E2BIG||errno==ENOENT||errno==ENOEXEC||errno==ENOMEM)//linux.pl/man/index.php?command=errno
        error("system error");
    system("gcc -o consumer consumer.c");
    if(errno==E2BIG||errno==ENOENT||errno==ENOEXEC||errno==ENOMEM)//linux.pl/man/index.php?command=errno
        error("system error");

*/

    if(argc !=4)//nazwa l.producentow l.konsumentow l.znakow
        error("zla liczba argumentow wywolania\n");
    
     if((strchr(argv[1],'-'))!=NULL)//znak
        error("liczba producentow: wystepuje znak '-'\n");
    
    unsigned long long producer_number=strtoull(argv[1],NULL,10);//string,NULL,number system
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");

    if((strchr(argv[2],'-'))!=NULL)
        error("liczba konsumentow: wystepuje znak '-'\n");
    
    unsigned long long consumer_number=strtoull(argv[2],NULL,10);//string,NULL,number system
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");
   
    if((strchr(argv[3],'-'))!=NULL)
        error("liczba znakow: wystepuje znak '-'\n");
    
    unsigned long  char_number=strtoul(argv[3],NULL,10);
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtol Erange error");
        
    //sprawdzenie czy nie podano za duzo znakow
    if(char_number>=PIPE_BUF)//4096
        error("za duzo znakow");

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
    long long processes_sum =existing_processes_number+producer_number+consumer_number;
    if(processes_sum>proc_limit_number)//sprawdzenie czy liczba uruchamianych procesow przez program nie przekroczy max liczby procesow
        error("Limit procesow zostal przekroczony ");
    
    int desc[2];//1-zapis, 0-odczyt
    if(pipe(desc)==-1)
        error("pipe error");
    else printf("lacze zostalo utworzone\n");
    printf("desc[0]=%d\tdesc[1]=%d\n",desc[0],desc[1]);

    //
    char read[15];
    char write[15];
    int chk;
    //desryptory
    chk=sprintf(read,"%d",desc[0]);
    if(chk<0)   error("sprintf error");
    chk=sprintf(write,"%d",desc[1]);
    if(chk<0)   error("sprintf error");
     //producer

    for(int i=0;i<producer_number;i++)
    {
        switch(fork())//tworzenie procesow potomnych, proces po fork przekazuje kopie deskryptorow do potomnych 
        {
            case -1:
                error("fork error");
            case 0:
           // printf("prod\n");
            if(execl("./producer","producer",argv[3],write,NULL)==-1)//odpalenie producenta arg wywolania to liczba znakow
                error("execl error");
            
            default:
                break;
        }
 
    }
      close(desc[1]);
    
    //sleep(3);
    //consumer
    for(int i=0;i<consumer_number;i++)
    {
        switch(fork())
        {
            case -1:
                error("fork error");
            case 0:
            //printf("kons\n");
            if(execl("./consumer","consumer",read,NULL)==-1)
                error("execl error");
            
            //sleep(2);
            default:
                break;
        }
        //zamkniecie procesow potomnych

    }
       close(desc[0]);
        int pid=0;
        int status =-1;
       for(int i=0; i<consumer_number+producer_number;i++)
       {

        int pid= wait(&status);
        if(status==-1)
        {
            error("wait error");
        }
        printf("proces : %d zakonczyl dzialanie. Kod powrotu :%d\n",pid,status);
       }

    return 0;
}