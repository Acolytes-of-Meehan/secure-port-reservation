/* Test the daemon */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include "spr.h"

int main (int argc, char **argv) {

  if (argc < 2) {
    printf("Usage: ./testRequest portNum\n");
    exit(EXIT_FAILURE);
  }
  
  int port = atoi(argv[1]
		  );
  char uds[PATH_MAX];
  time_t t;
  int mySocket;
  sprFDSet reservedPort;
  memset(uds, 0, PATH_MAX);
  strcpy(uds, "proc");
  srand((unsigned)time(&t));

  sprintf(&uds[4], "%dr%d", getpid(), rand() % 100);

  printf("Ready to secure_bind\n");

  if ((secure_bind(port, uds, &reservedPort)) < 0) {
    fprintf(stderr, "secure_bind returned with errno: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  printf("Ready to listen\n");

  if ((listen(reservedPort.recvSock, 5)) < 0) {
    fprintf(stderr, "listen errno: %d\n", errno);
  }

  struct sockaddr_in peer;
  socklen_t len = sizeof(peer);

  printf("Ready to accept\n");
  
  if ((mySocket = accept(reservedPort.recvSock, (struct sockaddr *)&peer, &len)) < 0) {
    fprintf(stderr, "accept errno: %d\n", errno);
  }

  char buf[20];
  memset(buf, 0, 20);
  strcpy(buf, "hello, world");

  if((send(mySocket, &buf, sizeof(buf), 0)) < 0) {
    fprintf(stderr, "send errno: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  char recvBuf[20];
  memset(recvBuf, 0, sizeof(recvBuf));
  
  if((recv(mySocket, recvBuf, sizeof(recvBuf), MSG_WAITALL)) < 0){
    fprintf(stderr, "recv errno%d\n", errno);
    exit(EXIT_FAILURE);
  }

  printf("%s\n", recvBuf);

  if ((secure_close(&reservedPort)) < 0) {
    fprintf(stderr, "secure_close returned with errno: %d\n", errno);
  }

  printf("No errors\n");
  
  return 0;

}
