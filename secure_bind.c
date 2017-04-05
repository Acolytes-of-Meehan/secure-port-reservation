/*
 * Benjamin Ellerby
 * Ray Luo
 * Evan Ricks
 * Oliver Smith-Denny
 * secure_bind.c function to receive a reserved port from Portcullis, the Secure Port Reservation Daemon
 * On success, returns a struct containing the file descriptor numbers of the received file descriptor and the unix domain socket 
 * On failure returns NULL
 *
 */

/* NOTE: May need to overload secure_bind(int portNum, sprFDSet *fds */

#include "spr.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/limits.h>
#include <errno.h>
#include <string.h>

#define PORT_MIN 0
#define PORT_MAX 65535
#define PORT_DIGITS 6
#define QLEN 5

extern errno;

sprFDSet * secure_bind(int portNum, char *udsPath){

  int recvFD, udsConnectSock, udsListenSock, localLen, remoteLen, flag;
  struct sockaddr_un local, remote;
  struct msghdr daemonPermissions, recvFDMsg;
  struct cmsghdr *daemonControl, *recvFDControl;
  char portBuf[PORT_DIGITS];
  /* Begin BSD incompatible */
  char cDataBuf[CMSG_SPACE(sizeof(struct ucred))];
  struct ucred *daemonCredentials;
  /* End BSD incompatible */
  sprFDSet returnSet;
  
  if(portNum < PORT_MIN || portNum > PORT_MAX) {
    errno = EINVAL;
    return NULL;
  }

  if(strlen(udsPath) >= PATH_MAX) {
    errno = ENAMETOOLONG;
    return NULL;
  }

  memset(&local, 0, sizeof(local));
  memset(&remote, 0, sizeof(remote));
  memset(&daemonPermissions, 0, sizeof(daemonPermissions));
  memset(cDataBuf, 0, sizeof(cDataBuf));
  memset(daemonCredentials, 0, sizeof(daemonCredentials));
  memset(portBuf, 0, sizeof(portBuf));
  memset(returnSet, 0, sizeof(returnSet));

  if((uds = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    return NULL;

  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, udsPath, PATH_MAX);
  unlink(local.sun_path);
  localLen = sizeof(local);

  if((listen(udsListenSock, QLEN)) < 0)
    return NULL;

  /* TODO: Write the pathname of the UDS Sock to the Daemon's named FIFO */

  /* Receive credentials from the Daemon and verify them (recvmsg) */

  remote.sun_family = AF_UNIX;
  strncpy(remote.sun_path, udsPath, PATH_MAX);
  remoteLen = sizeof(remote);

  if((udsConnectSock = accept(udsListenSock, (struct sockaddr *)&remote, &remoteLen)) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  flag = 1;
  if((setsockopt(udsConnectSock, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  daemonPermissions.msg_control = cDataBuf;
  daemonPermissions.msg_controllen = sizeof(cDataBuf);

  if((recvmsg(udsConnectSock, &daemonPermissions, MSG_WAITALL)) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  daemonControl = CMSG_FIRSTHDR(&daemonPermissions);

  if(daemonControl == NULL || daemonControl-<cmsg_type != SCM_CREDENTIALS) {
    errno = ENOMSG;
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  /* Begin BSD incompatible */

  daemonCredentials = (struct ucred *)CMSG_DATA(daemonControl);

  /* TODO: If it is not Daemon process, drop connection, goto line 74 (just kidding), exit or loop back */

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  /* TEMPORARY for debugging until daemon can have its own uid */

  printf("Received credentials were:\npid: %ld\nuid: %ld\ngid: %ld\n", (long)daemonCredentials->pid, (long)daemonCredentials->uid, (long)daemonCredentials->gid);
  
  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  /* End BSD incompatible */

  /* Send desired portNum to Daemon (send) */

  snprintf(portBuf, sizeof(portBuf), "%d", portNum);

  if((send(udsConnectSock, portBuf, sizeof(portBuf), 0)) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  /* Remove SO_PASSCRED from connected socket, no longer needed */

  flag = 0;
  if((setsockopt(udsConnectSock, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  /* Receive (recvmsg) the file descriptor or null from the Daemon */

  if((recvmsg(udsConnectSock, &recvFDMsg, MSG_WAITALL)) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  recvFDControl = CMSG_FIRSTHDR(&recvFDMsg);

  /* If null or no file descriptor, EXIT_FAILURE */

  if(recvFDControl == NULL || recvFDControl->cmsg_type != SCM_RIGHTS) {
    close(udsListenSock);
    close(udsConnectSock);
    return NULL;
  }

  /* Else, return the received file descriptor (recvFD) */

  memcpy(&recvFD, CMSG_DATA(recvFDControl), sizeof(recvFD));

  returnSet.recvSock = recvFD;
  returnSet.udsListen = udsListenSock;
  returnSet.udsConnect = udsConnectSock;
    
  return returnSet;
  
}
