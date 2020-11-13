#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void proces()
{
  printf("PID: %d PPID: %d UID: %d GID: %d\n", getpid(),getppid(),getuid(),getgid());
}

int main()
{
    proces();
    pid_t pid = getpid();
    char cmd[30];
    sprintf(cmd,"pstree -p %d", pid);
    for (int i = 0; i < 3; i++) 
    {
        switch (fork()) 
        {
        case 0://child process
        fork();
        fork();
            proces();
            break;
        case -1:
            perror("fork Error!\n");
            break;
        default://parent process
        fork();
            break;
        }
    }
 
    
    if (pid == getpid()) 
    {
        system(cmd);
    }
    sleep(1);
        return 0;
}

