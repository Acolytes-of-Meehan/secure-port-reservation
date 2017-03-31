#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main () {

    pid_t pid, sid;

    // Fork off of the parent process
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid < 0) {
        // Parent received a valid PID; exit parent process
        exit(EXIT_SUCCESS);
    }

    /* ------ IN CHILD PROCESS ------ */

    // Change the file mode mask
    umask(0);

    // TODO: open log file(s) here

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        // TODO: log failure
        exit(EXIT_FAILURE);
    }

    // Change the current working directory to root
    if (chdir("/") < 0) {
        // TODO: log failure
        exit(EXIT_FAILURE);
    }

    // Close the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Sleep forever
    while(1)
        sleep(60);

}
