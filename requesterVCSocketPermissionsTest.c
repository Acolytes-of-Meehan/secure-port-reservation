// Oliver Smith-Denny
// Unix Domain Socket and Permission Test -- Requester
// Part of Secure Port Reservation
// 2/15/17

#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

int main () {

  int recvFd, connectSock, udsConnectSock;
  struct sockaddr_un local, remote;
  struct msghdr msg;
  int len;
  int uds;
  int dummyFd = open("/home/smithdo/CS492/proj/tests/vc/test.txt", O_RDONLY);
  int dummyFd2 = open("/home/smithdo/CS492/proj/tests/vc/test2.txt", O_RDONLY);

  printf("My credentials are:\npid: %ld\nuid: %ld\ngid: %ld\n", (long)getpid(), (long)getuid(), (long)getgid());
  
  if ((uds = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "socket creation error");
    exit(EXIT_FAILURE);
  }

  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, "/home/smithdo/CS492/proj/tests/vc/uds");
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family);
  
  if((bind(uds, (struct sockaddr *)&local, len)) < 0) {
    fprintf(stderr, "bind error\n");
    exit(EXIT_FAILURE);
  }

  int flag1 = 1;
  if((setsockopt(uds, SOL_SOCKET, SO_REUSEADDR, &flag1, sizeof(int))) < 0){
    fprintf(stderr, "sock opt reuse error");
    exit(EXIT_FAILURE);
  }

  if((listen(uds, 5)) < 0) {
    fprintf(stderr, "listen error\n");
    exit(EXIT_FAILURE);
  }

  remote.sun_family = AF_UNIX;
  memset(remote.sun_path, 0, sizeof(remote.sun_path));
  strcpy(remote.sun_path, "/home/smithdo/CS492/proj/tests/vc/uds");
  int remoteLen = sizeof(remote);
  
  if((udsConnectSock = accept(uds, (struct sockaddr *)&remote, &remoteLen)) < 0) {
    fprintf(stderr, "accept error\n");
    exit(EXIT_FAILURE);
  }

  int flag2 = 1;
  if((setsockopt(udsConnectSock, SOL_SOCKET, SO_REUSEADDR, &flag2, sizeof(int))) < 0){
    fprintf(stderr, "sock opt reuse error");
    exit(EXIT_FAILURE);
  }

  struct msghdr permissions;
  struct iovec iov;
  int junk = 5;
  memset(&iov, 0, sizeof(iov));
  memset(&permissions, 0, sizeof(permissions));

  permissions.msg_iov = &iov;
  permissions.msg_iovlen = 1;
  struct cmsghdr *passCred;
  struct ucred cred;
  memset(&passCred, 0, sizeof(passCred));
  memset(&cred, 0, sizeof(cred));
  cred.pid = (long)getpid();
  cred.uid = (long)getuid();
  cred.gid = (long)getgid();
  char credBuf[CMSG_SPACE(sizeof(cred))];
  memset(credBuf, 0, sizeof(credBuf));
  iov.iov_base = &junk;
  iov.iov_len = sizeof(int);
  permissions.msg_control = credBuf;
  permissions.msg_controllen = sizeof(credBuf);
  passCred = CMSG_FIRSTHDR(&permissions);
  passCred->cmsg_level = SOL_SOCKET;
  passCred->cmsg_type = SCM_CREDENTIALS;
  passCred->cmsg_len = CMSG_LEN(sizeof(credBuf));
  memcpy(CMSG_DATA(passCred), &cred, sizeof(cred));
  permissions.msg_controllen = passCred->cmsg_len;
  permissions.msg_name = NULL;
  permissions.msg_namelen = 0;

  /*int flag = 1;
  
  if((setsockopt(udsConnectSock, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0){
    fprintf(stderr, "sock option error\n");
    exit(EXIT_FAILURE);
    }*/

  if((sendmsg(udsConnectSock, &permissions, 0)) < 0) {
    fprintf(stderr, "sendmsg error\n");
    exit(EXIT_FAILURE);
    }

  memset(&msg,   0, sizeof(msg));
  char cmsgbuf[CMSG_SPACE(sizeof(int))];
  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  msg.msg_control = cmsgbuf; // make place for the ancillary message to be received
  msg.msg_controllen = sizeof(cmsgbuf);

  if((recvmsg(udsConnectSock, &msg, MSG_WAITALL)) < 0) {
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
