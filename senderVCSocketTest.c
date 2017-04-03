// Oliver Smith-Denny
// Unix Domain Socket Test -- Sender
// Part of Secure Port Reservation
// 2/15/17

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

int main() {

  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sockInfo;
  sockInfo.sin_family = AF_INET;
  sockInfo.sin_port = htons(46799);
  int flagd = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flagd, sizeof(int));

  if((bind(fd, (struct sockaddr *)&sockInfo, sizeof(sockInfo))) < 0) {
    fprintf(stderr, "first sock bind err\n");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_un remote;
  int sendSock = socket(AF_UNIX, SOCK_DGRAM, 0);
  int len;

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, "/home/smithdo/CS492/proj/tests/vc/uds");
  len = strlen(remote.sun_path) + sizeof(remote.sun_family);
  
  if((connect(sendSock, (struct sockaddr *)&remote, len)) < 0) {
    fprintf(stderr, "Connect fail\n");
    exit(EXIT_FAILURE);
  }

  
  struct msghdr msg;
  struct iovec data;
  int tempData = 5;
  
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
  msg.msg_iov = &data;
  msg.msg_iovlen = 1;
  data.iov_base = &tempData;
  data.iov_len = sizeof(int);
  
  if((sendmsg(sendSock, &msg, 0)) < 0) {
    fprintf(stderr, "Could not send\n");
    exit(EXIT_FAILURE);
  }

  printf("Sent message\n");

  return 0;

}
