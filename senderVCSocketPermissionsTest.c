// Oliver Smith-Denny
// Unix Domain Socket and Permission Test -- Sender
// Part of Secure Port Reservation
// 2/15/17

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <string.h>

int main() {

  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sockInfo;
  sockInfo.sin_family = AF_INET;
  sockInfo.sin_port = htons(46799);

  if((bind(fd, (struct sockaddr *)&sockInfo, sizeof(sockInfo))) < 0) {
    fprintf(stderr, "first sock bind err\n");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_un remote;
  int sendSock = socket(AF_UNIX, SOCK_STREAM, 0);
  int len;

  int flag = 1;
  if((setsockopt(sendSock, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0){
    fprintf(stderr, "set sock opt error\n");
    exit(EXIT_FAILURE);
  }

  remote.sun_family = AF_UNIX;
  memset(remote.sun_path, 0, sizeof(remote.sun_path));
  strcpy(remote.sun_path, "/home/smithdo/CS492/proj/tests/vc/uds");
  len = strlen(remote.sun_path) + sizeof(remote.sun_family);

  int flag1 = 1;
  if((setsockopt(sendSock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int))) < 0){
    fprintf(stderr, "sock opt reuse error");
    exit(EXIT_FAILURE);
  }

  if((bind(sendSock, (struct sockaddr *)&remote, len)) < 0){
    fprintf(stderr, "bind error 2\n");
    exit(EXIT_FAILURE);
  }
  
  if((connect(sendSock, (struct sockaddr *)&remote, len)) < 0) {
    fprintf(stderr, "Connect fail\n");
    exit(EXIT_FAILURE);
  }
  
  struct msghdr credMsg;
  memset(&credMsg, 0, sizeof(credMsg));
  struct cmsghdr *cData;
  char cDataBuf[CMSG_SPACE(sizeof(struct ucred))];
  memset(cDataBuf, 0, sizeof(cDataBuf));
  credMsg.msg_control = cDataBuf;
  credMsg.msg_controllen = sizeof(cDataBuf);
  struct iovec inData;
  credMsg.msg_iov = &inData;
  credMsg.msg_iovlen = 1;
  int junk = 6;
  inData.iov_base = &junk;
  inData.iov_len = sizeof(int);

  if((recvmsg(sendSock, &credMsg, MSG_WAITALL)) < 0) {
    fprintf(stderr, "recvmsg error\n");
    exit(EXIT_FAILURE);
  }

  struct cmsghdr *passCred;
  struct ucred *realCred;
  passCred = CMSG_FIRSTHDR(&credMsg);
  
  if(passCred == NULL || passCred->cmsg_type != SCM_CREDENTIALS){
    fprintf(stderr, "No credentials sent\n");
    exit(EXIT_FAILURE);
  }

  realCred = (struct ucred *)CMSG_DATA(passCred);
  printf("Received Credentials were:\npid: %ld\nuid: %ld\ngid: %ld\n", (long)realCred->pid, (long)realCred->uid, (long)realCred->gid);
  
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
