#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int x;
    int execlStatus;

    for (int i = 0; i < 3; i++)
     {
        switch (fork()) 
        {
        case 0:
        //example of using execl
          execlStatus =  execl("/usr/bin/xterm","xterm","-bg","yellow","-fg","red",NULL); 
            if(execlStatus==-1)
            {
                perror("exelc ERROR");
                exit(1);
            }
            break;
        case -1:
            perror("fork ERROR!");
            exit(2);
            break;
        default:
            break;
        }
    }
  
    pid_t pid = getpid();
    char cmd[30];
    sprintf(cmd, "pstree -p %d", pid);
    system(cmd);
    
    for(int j=0;j<3;j++)
    {
        pid_t status=wait(&x);
        if(status==-1)
        {
            perror("wait ERROR!\n");
            exit(1);
        }
        else printf("funkcja wait zakonczyla proces %d ze statusem %d\n",status,x);
    }
        return 0;
}
