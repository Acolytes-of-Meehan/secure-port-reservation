/* sprd.c
 *
 * Ben Ellerby
 * Ray Weiming Luo
 * Evan Ricks
 * Oliver Smith-Denny
 *
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <linux/limits.h>
#include <string.h>
#include <syslog.h>

#define RETURN_SUCCESS 0
#define RETURN_FAILURE -1
#define PORT_DIGIT_MAX 5
#define NAMED_FIFO "~/tmp/spr_fifo"

int handleNewConnection (int namedFifo, fd_set *active_fdset);
int handleExistingConnection (int uds, char *portBuf);

int main () {

  int uds, i, namedFifo;
  pid_t pid, sid;
  fd_set active_fdset, read_fdset;
  char portBuf[PORT_DIGIT_MAX + 1];

  // Fork off of the parent process
  pid = fork();

  if (pid < 0) {
    // TODO: do we need this perror?
    perror("fork failed");
    // TODO: should this be a LOG_CRIT or LOG_ERR?
    syslog(LOG_MAKEPRI(LOG_DAEMON, LOG_CRIT), "Unable to fork daemon child process");
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    // Parent received a valid PID; exit parent process
    exit(EXIT_SUCCESS);
  }

  /* ------ IN CHILD PROCESS ------ */

  // Change the file mode mask
  umask(0);

  // Open syslog socket and set facility to LOG_DAEMON for future syslogs
  openlog("sprd", 0, LOG_DAEMON);

  // Create a new SID for the child process
  sid = setsid();
  if (sid < 0) {
    // TODO: should this be a LOG_CRIT or LOG_ERR?
    syslog(LOG_CRIT, "Unable to run daemon child process in a new session");
    exit(EXIT_FAILURE);
  }

  // Change the current working directory to root
  if (chdir("/") < 0) {
    // TODO: should this be a LOG_CRIT or LOG_ERR?
    syslog(LOG_CRIT, "Unable to change daemon process's current working directory to '/'");
    exit(EXIT_FAILURE);
  }

  // Zero active file descriptors for select
  FD_ZERO(&active_fdset);

  // Close the standard file descriptors
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  //close(STDERR_FILENO);

  // TODO: parse through config file to generate a linked list of reservations

  // Unlink named fifo
  // NOTE: Check ernno from unlink() for possible return errors
  
  if (unlink(NAMED_FIFO) < 0) {
    if (errno != ENOENT) {
        //TODO: log failure
    }
  }
 
  // Use chmod(2) for:
  // WRITE: S_IWUSR (for current testing only), S_IWOTH and S_IWGRP
  // READ: S_IRUSR
  // TODO: Change to O_RDONLY after testing
  if((namedFifo = mkfifo(NAMED_FIFO, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH)) < 0) {
    //TODO: LOG Failure
  }

  // TODO: infinite loop listening on FIFO for secure bind/close requests
  while(1) {
    sleep(30);
    // TODO: Get uds socket then FD_SET the uds socket into the active_fset
    read_fdset = active_fdset;
    if (select (FD_SETSIZE, &read_fdset, NULL, NULL, NULL) == 0) {
      for (i = 0; i < FD_SETSIZE; i++) {
	if (FD_ISSET(i, &read_fdset)) {
	  switch (i) {
	  case namedFifo :
	    /* run secure_bind interface, read uds name from named fifo, try to connect to that uds */
	    handleNewConnection(namedFifo, &active_fdset);
	    break;
	    /* This is the case where it is not the named fifo, so it is one of possibly many ud sockets */
	    /* Need to check what was sent across, if 0, shutdown connection and FD_CLR, if not 0, then they have sent credentials/port request */
	  default :
	    fprintf(stderr, "SELECT: %d", uds); //test
	    memset(portBuf, 0, sizeof(portBuf));
	    if ((recv(i, portBuf, PORT_DIGIT_MAX, MSG_PEEK)) < 0) {
	      //TODO: log error, recv failure
	      break;
	    } else if (strlen(portBuf) == 0) {
	      /* This is the case of secure_close, i can be shutdown and FD_CLR'ed from active_fdset */
            if (shutdown(i, SHUT_RDWR) < 0) {
                //TODO: log failure
            }

            FD_CLR(i, &active_fdset);
	    } else {
	      /* This is the case of secure_bind */
	      handleExistingConnection(i, portBuf);
	    }

	  }
	}
      } else {
	// TODO: log failure
      }
    }
  }
}

/* Function to handle a request on a unix domain socket, return 0 on success, -1 on failure */

int handleExistingConnection (int uds, char *portBuf) {

  struct msghdr credMsg;
  struct cmsghdr *passCred;
  /* Begin BSD incompatible */
  char cDataBuf[CMSG_SPACE(sizeof(struct ucred))];
  struct ucred *realCred;
  /* End BSD incompatible */
  struct iovec inData;
  int flag = 1;

  memset(credMsg, 0, sizeof(credMsg));
  memset(cDataBuf, 0, sizeof(cDataBuf));
  memset(inData, 0, sizeof(inData));
  memset(portBuf, 0, sizeof(portBuf));

  credMsg.msg_control = cDataBuf;
  credMsg.msg_controllen = sizeof(cDataBuf);
  credMsg.msg_iov = &inData;
  credMsg.msg_iovlen = 1; //may need to be changed from 1 to sizeof(portBuf)
  inData.iov_base = portBuf;
  inData.iov_len = sizeof(portBuf);

  if((setsockopt(uds, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0){
    //TODO: log error, setsockopt error
    return RETURN_FAILURE;
  }

  if((recvmsg(uds, &credMsg, 0)) < 0){
    //TODO: log error, recvmsg fail
    return RETURN_FAILURE;
  }

  passCred = CMSG_FIRSTHDR(&credMsg);

  if(passCred == NULL || passCred->cmsg_type != SCM_CREDENTIALS){
    //TODO: log error, did not receive credentials
    return RETURN_FAILURE;
  }

  realCred = (struct ucred *)CMSG_DATA(passCred);

  flag = 0;

  if((setsockopt(uds, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0){
    //TODO: log error, setsockopt error
    exit(EXIT_FAILURE);
  }

  //TODO: check data structure, pass fd if ok, otherwise don't pass

  return RETURN_SUCCESS;

}

/* Function to handle a request on the named FIFO, returns 0 on success, -1 on failure */

int handleNewConnection (int namedFifo, fd_set *active_fdset) {

  char readBuf[PATH_MAX + 1];
  char sendChar = 'd';
  struct sockaddr_un remote;
  int len, connectSock;

  memset(readBuf, 0, sizeof(readBuf));
  memset(remote, 0, sizeof(remote));

  if((read(namedFifo, readBuf, PATH_MAX)) == 0) {
    //TODO: log error, read nothing from fifo
    return RETURN_FAILURE;
  }

  if((sendSock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    //TODO: log error, socket call failed
    return RETURN_FAILURE;
  }
  remote.sun_family = AF_UNIX;
  strncpy(remote.sun_path, readBuf, strlen(readBuf));
  len = sizeof(remote);

  if((connect(connectSock, (struct sockaddr *)&remote, len)) < 0) {
    //TODO: log error, connect failed
    return RETURN_FAILURE;
  }

  if((send(connectSock, &sendChar, sizeof(sendChar), 0)) < 0) {
    //TODO: log error, could not send
    return RETURN_FAILURE;
  }

  /* Add to active_fdset to monitor */
  FD_SET(active_fdset, connectSock);

  return RETURN_SUCCESS;
}
