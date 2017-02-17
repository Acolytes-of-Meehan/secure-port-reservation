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
  
  /* file descriptor received from another process, active internet socket, length of v'ble local, and unix domain socket, respectively */
  int recvFd, connectSock, len, uds;
  struct sockaddr_un local, remote;
  
  /* space to receive message from unix domain socket */
  struct msghdr msg;
  
  /* two dummy file descriptors, used to ensure this process has a different integer value for the received file descriptor than sending process */
  int dummyFd = open("/home/smithdo/CS492/proj/tests/datagram/test.txt", O_RDONLY);
  int dummyFd2 = open("/home/smithdo/CS492/proj/tests/datagram/test2.txt", O_RDONLY);

  /* create the unix domain socket as a datagram socket */
  if ((uds = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "socket creation error");
    exit(EXIT_FAILURE);
  }

  /* fill in local information */
  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, "/home/smithdo/CS492/proj/tests/datagram/uds");
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family);

  /* bind unix domain socket to local info */
  if((bind(uds, (struct sockaddr *)&local, len)) < 0) {
    fprintf(stderr, "bind error");
    exit(EXIT_FAILURE);
  }

  /* initalize space for received message from unix domain socket */
  memset(&msg,   0, sizeof(msg));
  char cmsgbuf[CMSG_SPACE(sizeof(int))];
  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  /* space for control data (file descriptor) to be received into */
  msg.msg_control = cmsgbuf; 
  msg.msg_controllen = sizeof(cmsgbuf);

  /* receive data from unix domain socket, wait for all of message */
  if((recvmsg(uds, &msg, MSG_WAITALL)) < 0) {
    fprintf(stderr, "recvmsg error");
    exit(EXIT_FAILURE);
  }

  /* retrieve control data from received message and ensure it contains a file descriptor */
  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  
  if (cmsg == NULL || cmsg -> cmsg_type != SCM_RIGHTS) {
    printf("The first control structure contains no file descriptor.\n");
    exit(0);
  }

  /* copy file descriptor into local variable, report receipt */
  memcpy(&recvFd, CMSG_DATA(cmsg), sizeof(recvFd));

  printf("file descriptor passed: %d\n", recvFd);

  /* listen on received socket */
  if((listen(recvFd, 5)) < 0){
    fprintf(stderr, "Listen Failure\n");
    exit(EXIT_FAILURE);
  }

  /* information about connected partner */
  struct sockaddr_in peer;
  int peerLen = sizeof(peer);

  /* accept a connection */
  if((connectSock = accept(recvFd, (struct sockaddr *)&peer, &peerLen)) < 0) {
    fprintf(stderr, "accept failure\n");
    exit(EXIT_FAILURE);
  }

  /* send test buffer to connected process */
  char buf[20];
  memset(buf, 0, 20);
  strcpy(buf, "hello, world");

  if((send(connectSock, &buf, sizeof(buf), 0)) < 0) {
    fprintf(stderr, "send failure\n");
    exit(EXIT_FAILURE);
  }
  /* receive test info (20 chars) from connected process and print */
  char recvBuf[20];
  memset(recvBuf, 0, sizeof(recvBuf));
  
  if((recv(connectSock, recvBuf, sizeof(recvBuf), MSG_WAITALL)) < 0){
    fprintf(stderr, "recv failure\n");
    exit(EXIT_FAILURE);
  }

  printf("%s\n", recvBuf);

  /* go back from whence you came */
  return 0;

}
