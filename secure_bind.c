/*
 * Benjamin Ellerby
 * Ray Luow
 * Evan Ricks
 * Oliver Smith-Denny
 * secure_bind.c function to receive a reserved port from Portcullis, the Secure Port Reservation Daemon
 *
 */

#define _GNU_SOURCE
#include "spr.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/limits.h>

int secure_bind(int portNum, char *udsPath){

  int recvFD, udsConnectSock, udsListenSock, localLen, remoteLen;
  struct sockaddr_un local, remote;
  struct msghdr daemonPermissions, recvFDMsg;
  struct cmsghdr daemonControl, recvFDControl;

  memset(local, 0, sizeof(local));
  memset(remote, 0, sizeof(remote));

  if((uds = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    return EXIT_FAILURE;
  }

  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, udsPath, PATH_MAX);
  unlink(local.sun_path);
    
  return 0;
  
}
