// Oliver Smith-Denny
// Unix Domain Socket Test -- Requester
// Part of Secure Port Reservation
// 2/15/17

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

int main () {

  int recvFd, connectSock;
  struct sockaddr_un local, remote;
  struct msghdr msg;
  int len;
  int uds;
  int dummyFd = open("/home/smithdo/CS492/proj/tests/vc/test.txt", O_RDONLY);
  int dummyFd2 = open("/home/smithdo/CS492/proj/tests/vc/test2.txt", O_RDONLY);
  if ((uds = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "socket creation error");
    exit(EXIT_FAILURE);
  }

  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, "/home/smithdo/CS492/proj/tests/vc/uds");
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family);
  
  if((bind(uds, (struct sockaddr *)&local, len)) < 0) {
    fprintf(stderr, "bind error");
    exit(EXIT_FAILURE);
  }

  memset(&msg,   0, sizeof(msg));
  char cmsgbuf[CMSG_SPACE(sizeof(int))];
  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  msg.msg_control = cmsgbuf; // make place for the ancillary message to be received
  msg.msg_controllen = sizeof(cmsgbuf);

  if((recvmsg(uds, &msg, MSG_WAITALL)) < 0) {
    fprintf(stderr, "recvmsg error");
    exit(EXIT_FAILURE);
  }
  
  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  
  if (cmsg == NULL || cmsg -> cmsg_type != SCM_RIGHTS) {
    printf("The first control structure contains no file descriptor.\n");
    exit(0);
  }
  
  memcpy(&recvFd, CMSG_DATA(cmsg), sizeof(recvFd));

  printf("file descriptor passed: %d\n", recvFd);

  if((listen(recvFd, 5)) < 0){
    fprintf(stderr, "Listen Failure\n");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in peer;
  int peerLen = sizeof(peer);
  
  if((connectSock = accept(recvFd, (struct sockaddr *)&peer, &peerLen)) < 0) {
    fprintf(stderr, "accept failure\n");
    exit(EXIT_FAILURE);
  }

  char buf[20];
  memset(buf, 0, 20);
  strcpy(buf, "hello, world");

  if((send(connectSock, &buf, sizeof(buf), 0)) < 0) {
    fprintf(stderr, "send failure\n");
    exit(EXIT_FAILURE);
  }

  char recvBuf[20];
  memset(recvBuf, 0, sizeof(recvBuf));
  
  if((recv(connectSock, recvBuf, sizeof(recvBuf), MSG_WAITALL)) < 0){
    fprintf(stderr, "recv failure\n");
    exit(EXIT_FAILURE);
  }

  printf("%s", recvBuf);
  
  return 0;

}
