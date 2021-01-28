#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <limits.h>
#include <wait.h>
#include <time.h>
extern void error(char*string);

int main(int argc,char* argv[])
{
    FILE* input;
    char file_name[30];
    char sign;
    int chk=sprintf(file_name,"we/we_%d",getpid());//plik wejsciowy nazwa
    if(chk<0)   error("sprintf error");
    int descriptor;
    if(( descriptor=strtol(argv[2],NULL,10))==0)
        error("strtol error");


    //nie  sprawdzam '-' bo sprawdzilem w pipe.c 
    unsigned long long char_limit = strtoull(argv[1],NULL,10);
    if(errno==ERANGE)//gdy nastapi przepelnienie wartosci strtoul errno ustawiane jest na ERANGE
        error("strtoull error");
    

    //otworzenie pliku do zapisu znakow
    if((input=fopen(file_name,"w"))==NULL)
        perror("file open error");
    //producent nie wypisuje jakie znaki pobral bo wyjscie ustawione jest na pipe
    for(int i=0;i<char_limit;i++)
    {
        sign=(rand()%26+'a');
        printf("producent wyprodukowal: %c\n",sign);
        if(fprintf(input,"%c",sign)<0)//zapis znakow do pliku
            error("fprintf error");        

        if((write(descriptor,&sign,sizeof(char)))==-1)//zapis znakow do potoku
            error("write error");
    }
    //zamkniecie pliku
    if(fclose(input)==1)
        error("fclose error");
        //zamkniecie deskryptora
    if(close(descriptor)==1)
        error("fclose error");
    return 0;
}