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

    /* TODO: initialize reservation struct with the following fields:
     *          busy - int; a non-zero value means the port has already been
     *                  securely 'bound' to by another process
     *          uids - array of ints representing a set of user ids
     *          gids - array of ints representing a set of group ids
     */

    /* TODO: initialize hash table (GHashTable from GLib?) that will be used to
     *          store port reservations
     */

    /* TODO: read from config file and for each port reservation listed:
     *          1. bind to port
     *          2. store listed uids and gids in a reservation struct
     *          3. insert entry into hash table with port number as key and
     *              reservation struct as value
     */

    // TODO: infinite loop listening on FIFO for secure bind/close requests
    while(1)
        sleep(60);

}
