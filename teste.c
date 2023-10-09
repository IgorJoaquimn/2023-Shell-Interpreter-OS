#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main()
{
    printf("teste.c ta rodando %d\n", getpid());
    pid_t forkrank = fork();
    if (forkrank == -1)
    {
        printf("erros %d\n", getpid());
        return -1;
    }
    if (forkrank == 0) // filho
    {
        char *args[2];
        args[0] = "/bin/ls";        // first arg is the full path to the executable
        args[1] = NULL;  
        execv( args[0], args );
    }
    else  // pai
        printf("de volta no teste.c %d\n", getpid());
    return 0;
}