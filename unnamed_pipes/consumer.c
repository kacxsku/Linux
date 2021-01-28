#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <limits.h>
#include <wait.h>
extern void error(char*string);
int main(int argc,char* argv[])
{
    FILE* output;
    char file_name[30];//nazwa pliku
    char buff;//bufor na znaki z pipe
    int grip;
    int chk;
    long descriptor;
    chk=sprintf(file_name,"wyj/wyj_%d",getpid());//zapis pliku do kat wyj o nazwie wyj_pid
    if(chk<0)   error("sprintf error");
    //otworzenie pliku

    if(( descriptor=strtol(argv[1],NULL,10))==0)
        error("strtol error");
   //printf("deskryptor : %d/n",descriptor);
    if ((output=fopen(file_name,"w"))==NULL)
        error("fopen error");

    while ((grip=read(descriptor,&buff,sizeof(char)))>0)//odczyt z potoku i zapis znaku do bufora
    {
        if(grip==-1)
            error("Konsumnet\tBlad przy czytaniu");
        printf("Konsument %d\todczytane dane:\t%c\n",getpid(),buff);
        if(fprintf(output,"%c",buff)<0)//zapis z bufora do pliku
            error("Konsument\tBlad zapisu do pliku");
    }


    //zamykanie pliku
    if(fclose(output)==1)
        error("Konsument\tBlad zamykania pliku wyjsciowego ");
    //zamkniecie deskryptora
    if(close(descriptor)==1)
        error("fclose error");
    return 0;
}