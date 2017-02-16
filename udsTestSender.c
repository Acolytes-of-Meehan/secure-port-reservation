// Oliver Smith-Denny
// Unix Domain Socket Test -- Sender
// Part of Secure Port Reservation
// 2/15/17

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

int main() {

  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in send;
  int length = sizeof(struct sockaddr_in);
  
  if((bind(fd, (struct sockaddr *)&send, length)) < 0) {
    fprintf(stderr, "bind error");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_un remote;
  int sendSock = socket(AF_UNIX, SOCK_DGRAM, 0);
  int len;

  /*remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, "/home/smithdo/CS492/uds");
  len = strlen(remote.sun_path) + sizeof(remote.sun_family);
  connect(sendSock, (struct sockaddr *)&remote, len);*/

  
  struct msghdr msg;
  
  memset(&msg, 0, sizeof(msg));
  struct cmsghdr *cmsg;
  char cmsgbuf[CMSG_SPACE(sizeof(fd))];
  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  msg.msg_control = cmsgbuf;
  msg.msg_controllen = sizeof(cmsgbuf); // necessary for CMSG_FIRSTHDR to return the correct value
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
  memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));
  msg.msg_controllen = cmsg->cmsg_len; // total size of all control blocks

  //struct cmsghdr *test = CMSG_FIRSTHDR(&msg);
  //int testFd;
  //memcpy(&testFd, CMSG_DATA(test), sizeof(testFd));

  //printf("testFd is %d\n", testFd);

  if((sendmsg(sendSock, &msg, 0)) < 0) {
    fprintf(stderr, "Could not send\n");
    exit(EXIT_FAILURE);
  }

  printf("Sent message\n");

  return 0;

}
