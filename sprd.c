/* sprd.c
 * 
 * Ben Ellerby
 * Ray Weiming Luo
 * Evan Ricks
 * Oliver Smith-Denny 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>


int main () {

    int uds;
    int i;
    pid_t pid, sid;
    fd_set active_fdset, read_fdset;

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

    // Zero active file descriptors for select
    FD_ZERO(&active_fdset);

    // Close the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    //close(STDERR_FILENO);

    // TODO: parse through config file to generate a linked list of reservations

    // TODO: infinite loop listening on FIFO for secure bind/close requests
    while(1) {
        sleep(30);
        // TODO: Get uds socket then FD_SET the uds socket into the active_fset
        read_fdset = active_fdset;
        if (select (FD_SETSIZE, &read_fdset, NULL, NULL, NULL) == 0) {
            for (i = 0; i < FD_SETSIZE; i++) {
                if (FD_ISSET(uds, &read_fdset)) {
                    fprintf(stderr, "SELECT: %d", uds);
                }
            }
        } else {
            // TODO: log failure
        }
    }

}
