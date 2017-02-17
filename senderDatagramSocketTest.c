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

  /* internet socket to be passed to another process */
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  /* bind to port 46799 */
  struct sockaddr_in sockInfo;
  sockInfo.sin_family = AF_INET;
  sockInfo.sin_port = htons(46799);

  if((bind(fd, (struct sockaddr *)&sockInfo, sizeof(sockInfo))) < 0) {
    fprintf(stderr, "first sock bind err\n");
    exit(EXIT_FAILURE);
  }

  /* connect to unix domain socket using datagram protocol */
  struct sockaddr_un remote;
  int sendSock = socket(AF_UNIX, SOCK_DGRAM, 0);
  int len;

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, "/home/smithdo/CS492/proj/tests/datagram/uds");
  len = strlen(remote.sun_path) + sizeof(remote.sun_family);
  
  if((connect(sendSock, (struct sockaddr *)&remote, len)) < 0) {
    fprintf(stderr, "Connect fail\n");
    exit(EXIT_FAILURE);
  }

  /* message to be passed to other process, containing file descriptor */
  struct msghdr msg;
  
  memset(&msg, 0, sizeof(msg));
  struct cmsghdr *cmsg;
  char cmsgbuf[CMSG_SPACE(sizeof(fd))];
  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  /* space for file descriptor to be passed in */
  msg.msg_control = cmsgbuf;
  msg.msg_controllen = sizeof(cmsgbuf);
  cmsg = CMSG_FIRSTHDR(&msg);
  /* use socket layer and indicate that an array contains access rights (for file descriptor) */
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
  /* package control message into sending message */
  memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));
  msg.msg_controllen = cmsg->cmsg_len; 

  /* send message containing file descriptor to other process */
  if((sendmsg(sendSock, &msg, 0)) < 0) {
    fprintf(stderr, "Could not send\n");
    exit(EXIT_FAILURE);
  }

  printf("Sent message containing file descriptor: %d\n", fd);

  return 0;

}
