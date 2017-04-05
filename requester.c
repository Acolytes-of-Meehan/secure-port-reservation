/*
 * Oliver Smith-Denny, et. al.
 * Testing secure_bind functionality, requester side
 * Calls secure_bind
 */

#include "spr.h"
#include <stdio.h>

int main(){

  int portNum = 36799;
  char *udsPath = "/home/smithdo/CS493/secure-port-reservation/myUDS";
  sprFDSet *mySet;

  secure_bind(portNum, udsPath, mySet);

  if(mySet == NULL)
    printf("Not Happy\n");
  else
    printf("My happy little statement that I got:\nmySet.recvSock: %d\nmySet.udsListen: %d\nmySet.udsConnect: %d",
	   mySet.recvSock, mySet.udsListen, mySet.udsConnect);
  
  return 0;

}
