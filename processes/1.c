
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
/* PID -  prosess ID 
   PPID - parent process ID 
   UID -  ID of user who stareted the process 
   GID -  ID of user's group*/

int main() {
  printf("PID: %d PPID: %d UID: %d GID: %d\n", getpid(),getppid(),getuid(),getgid());
  return 0;
}