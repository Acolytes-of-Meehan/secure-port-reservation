/* secure_bind.c
 *
 * Benjamin Ellerby
 * Ray Luo
 * Evan Ricks
 * Oliver Smith-Denny
 *
 * secure_bind.c function to receive a reserved port from Portcullis, the Secure Port Reservation Daemon
 * On success, returns a 0, returnSet is filled out with relevant information 
 * On failure returns -1, returnSet is not defined
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
#define RETURN_FAILURE -1
#define RETURN_SUCCESS 0

int secure_bind(int portNum, char *udsPath, sprFDSet *returnSet){

  int recvFD, udsConnectSock, udsListenSock, localLen, remoteLen, flag;
  struct sockaddr_un local, remote;
  struct msghdr daemonPermissions, recvFDMsg;
  struct cmsghdr *daemonControl, *recvFDControl;
  char portBuf[PORT_DIGITS];
  /* Begin BSD incompatible */
  char cDataBuf[CMSG_SPACE(sizeof(struct ucred))];
  struct ucred dCred;
  struct ucred *daemonCredentials = &dCred;
  /* End BSD incompatible */
  
  if(portNum < PORT_MIN || portNum > PORT_MAX) {
    errno = EINVAL;
    return RETURN_FAILURE;
  }

  if(strlen(udsPath) >= PATH_MAX) {
    errno = ENAMETOOLONG;
    return RETURN_FAILURE;
  }

  memset(&local, 0, sizeof(local));
  memset(&remote, 0, sizeof(remote));
  memset(&daemonPermissions, 0, sizeof(daemonPermissions));
  memset(cDataBuf, 0, sizeof(cDataBuf));
  memset((void *)daemonCredentials, 0, sizeof(*daemonCredentials));
  memset(portBuf, 0, sizeof(portBuf));
  memset((void *)returnSet, 0, sizeof(*returnSet));

  if((udsListenSock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    return RETURN_FAILURE;

  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, udsPath, strlen(udsPath));
  unlink(local.sun_path);
  localLen = sizeof(local);

  if((bind(udsListenSock, (struct sockaddr *)&local, localLen)) < 0)
    return RETURN_FAILURE;

  if((listen(udsListenSock, QLEN)) < 0)
    return RETURN_FAILURE;

  /* TODO: Write the pathname of the UDS Sock to the Daemon's named FIFO */

  /* Receive credentials from the Daemon and verify them (recvmsg) */

  remote.sun_family = AF_UNIX;
  strncpy(remote.sun_path, udsPath, strlen(udsPath));
  remoteLen = sizeof(remote);

  if((udsConnectSock = accept(udsListenSock, (struct sockaddr *)&remote, (socklen_t *)&remoteLen)) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return RETURN_FAILURE;
  }

  flag = 1;
  if((setsockopt(udsConnectSock, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return RETURN_FAILURE;
  }

  daemonPermissions.msg_control = cDataBuf;
  daemonPermissions.msg_controllen = sizeof(cDataBuf);

  if((recvmsg(udsConnectSock, &daemonPermissions, MSG_WAITALL)) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return RETURN_FAILURE;
  }

  daemonControl = CMSG_FIRSTHDR(&daemonPermissions);

  if(daemonControl == NULL || daemonControl->cmsg_type != SCM_CREDENTIALS) {
    errno = ENOMSG;
    close(udsListenSock);
    close(udsConnectSock);
    return RETURN_FAILURE;
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
    return RETURN_FAILURE;
  }

  /* Remove SO_PASSCRED from connected socket, no longer needed */

  flag = 0;
  if((setsockopt(udsConnectSock, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return RETURN_FAILURE;
  }

  /* Receive (recvmsg) the file descriptor or null from the Daemon */

  if((recvmsg(udsConnectSock, &recvFDMsg, MSG_WAITALL)) < 0) {
    close(udsListenSock);
    close(udsConnectSock);
    return RETURN_FAILURE;
  }

  recvFDControl = CMSG_FIRSTHDR(&recvFDMsg);

  /* If null or no file descriptor, EXIT_FAILURE */

  if(recvFDControl == NULL || recvFDControl->cmsg_type != SCM_RIGHTS) {
    close(udsListenSock);
    close(udsConnectSock);
    return RETURN_FAILURE;
  }

  /* Else, return the received file descriptor (recvFD) */

  memcpy(&recvFD, CMSG_DATA(recvFDControl), sizeof(recvFD));

  returnSet->recvSock = recvFD;
  returnSet->udsListen = udsListenSock;
  returnSet->udsConnect = udsConnectSock;

  fprintf(stdout, "%d, %d, %d\n", recvFD, udsListenSock, udsConnectSock);
    
  return RETURN_SUCCESS;
  
}
