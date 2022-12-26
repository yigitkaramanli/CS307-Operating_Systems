#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{

    printf("I'm SHELL process, with PID: %d - Main command is: \"man grep | grep \"-A\" -A 3 \" \n ", getpid());
    int fd[2];
    pid_t cpid;

    if (pipe(fd) < 0)
    {
        perror("pipe");
        exit(1);
    }

    cpid = fork();
    if (cpid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (cpid == 0)
    {
        // MAN process
        printf("I'm MAN process, with PID: %d - My command is: \"man grep\"\n", getpid());

        //dup2(fd[1],STDOUT_FILENO);
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) < 0)
        {
            perror("dup2");
            exit(1);
        }

        char *myargs[3];
        myargs[0] = strdup("man");
        myargs[1] = strdup("grep");
        myargs[2] = NULL;

        execvp(myargs[0], myargs);
    }

    wait(NULL);

    cpid = fork();

    if (cpid < 0)
    {
        perror("fork");
        exit(1);
    }

    else if (cpid == 0)
    {
        // GREP process
        printf("I'm GREP process, with PID: %d - My command is: \"grep \"-A\" -A 3\"\n", getpid());
        int new_fd = open("output.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        close(fd[1]);
        if (dup2(fd[0], STDIN_FILENO) < 0)
        {
            perror("dup2");
            exit(1);
        }
        if (dup2(new_fd, STDOUT_FILENO) < 0)
        {
            perror("dup2");
            exit(1);
        }

        char *myargs[5];
        myargs[0] = "grep";
        myargs[1] = "\\-A";
        myargs[2] = "-A";
        myargs[3] = "3";
        myargs[4] = NULL;

        // char *myargs[3]; was just here for testing purposes
        // myargs[0] = "grep";
        // myargs[1] = "\\-c";
        // myargs[2] = NULL;

        execvp(myargs[0], myargs);
    }
    // returned to parent process
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    
    printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt\n", getpid());
    
    return 0;
}