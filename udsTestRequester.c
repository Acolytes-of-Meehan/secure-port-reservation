// Oliver Smith-Denny
// Unix Domain Socket Test -- Requester
// Part of Secure Port Reservation
// 2/15/17

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main () {

  int recvFd, connectSock;
  struct sockaddr_un local, remote;
  struct msghdr msg;
  int len;

  int uds;
  if ((uds = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "socket creation error");
    exit(EXIT_FAILURE);
  }

  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, "/home/smithdo/CS492/uds");
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family);
  if((bind(uds, (struct sockaddr *)&local, len)) < 0) {
    fprintf(stderr, "bind error");
    exit(EXIT_FAILURE);
  }

  /* if((listen(uds, 5)) < 0) {
    fprintf(stderr, "listen error");
    exit(EXIT_FAILURE);
  }*/

  /* len = sizeof(struct sockaddr_un);
  if((connectSock = accept(uds, (struct sockaddr *)&remote, &len)) < 0) {
    fprintf(stderr, "accept error");
    exit(EXIT_FAILURE);
  }*/

  memset(&msg,   0, sizeof(msg));
  char cmsgbuf[CMSG_SPACE(sizeof(int))];
  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  msg.msg_control = cmsgbuf; // make place for the ancillary message to be received
  msg.msg_controllen = sizeof(int);

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

  write(recvFd, "file descriptor passed", 23);
  
  return 0;

}
