/*
 * Oliver Smith-Denny, et. al.
 * Testing secure_bind functionality, requester side
 * Calls secure_bind
 */

#include "spr.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int main(){

  printf("My credentials are:\npid: %ld\nuid: %ld\ngid: %ld\n", (long)getpid(), (long)getuid(), (long)getgid());

  int portNum = 36799;
  char *udsPath = "/home/smithdo/CS493/secure-port-reservation/myUDS";
  sprFDSet mySet;

  if((secure_bind(portNum, udsPath, &mySet)) != 0) {
    fprintf(stderr, "Gee willickers, that's not good. Errno: %d\n", errno);
    exit(EXIT_FAILURE);
  }
    
  printf("My happy little statement that I got:\nmySet.recvSock: %d\nmySet.udsListen: %d\nmySet.udsConnect: %d\n",
	 mySet.recvSock, mySet.udsListen, mySet.udsConnect);
  
  return 0;

}
