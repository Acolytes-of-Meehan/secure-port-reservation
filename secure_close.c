/* secure_close.c
 *
 * Ben Ellerby
 * Ray Weiming Luo
 * Evan Ricks
 * Oliver Smith-Denny
 *
 * Close the udsListen and udsConnect (domain file descriptor) 
 * which is handled by the kernel. And close the recvSock (TCP socket)
 * that is a socket handed from the server to the specified
 * client. Return 0 is successfull, otherwise return -1.
 *
 */

#define _GNU_SOURCE
#include "spr.h"
#include <unistd.h>

int secure_close (sprFDSet *closeSet) {
    return (close(closeSet->udsConnect) | close(closeSet->udsListen) | close(closeSet->recvSock));
}
