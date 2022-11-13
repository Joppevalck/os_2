#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int status;

    char* lsargs[3];
    lsargs[0] = "/bin/ls";
    lsargs[1] = "/";
    lsargs[2] = NULL;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return -1;
    }    
    
    if ( fork() == 0 ){     // Child process
        close(pipefd[0]);

        dup2(pipefd[1], STDOUT_FILENO); // stdout
        dup2(pipefd[1], STDERR_FILENO); // stderr

        close(pipefd[1]);

        execv(lsargs[0], lsargs);     // call ls with / as arg
    } else {                // Parent process
        close(pipefd[1]);
        wait( &status );        // parent: wait for the child (not really necessary)

        dup2(pipefd[0], STDIN_FILENO);

        char* wcargs[4];
        wcargs[0] = "/usr/bin/wc"; // which wc
        wcargs[1] = "-l";
        wcargs[2] = NULL;
       
        execv(wcargs[0], wcargs);
        
        close(pipefd[0]);
    }

    return 0;
}