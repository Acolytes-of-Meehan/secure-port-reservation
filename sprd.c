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
#include "parse_config.h"

#define RETURN_SUCCESS 0
#define RETURN_FAILURE -1
#define PORT_DIGIT_MAX 5
#define NUM_PORTS 65536
#define NAMED_FIFO "~/tmp/spr_fifo"
#define CONFIG_FILE "sprd.conf"

int handleNewConnection (int namedFifo, fd_set *active_fdset);
int handleExistingConnection (int uds, char *portBuf);

int main () {

  int i, namedFifo, fd;
  pid_t pid, sid;
  fd_set active_fdset, read_fdset;
  char portBuf[PORT_DIGIT_MAX + 1];
  int sockList[NUM_PORTS][2]; // sockList[portNum][0] --> inUse, sockList[portNum][1] --> fd for socket
  res* r;

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
    syslog(LOG_CRIT, "Unable to run daemon child process in a new session");
    exit(EXIT_FAILURE);
  }

  // Change the current working directory to root
  if (chdir("/") < 0) {
    syslog(LOG_CRIT, "Unable to change daemon process's current working directory to '/'");
    exit(EXIT_FAILURE);
  }

  // Zero active file descriptors for select
  FD_ZERO(&active_fdset);

  // Close the standard file descriptors
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  //close(STDERR_FILENO);

  // Parse config file to generate a linked list of reservations
  // IMPORTANT: change this path to point to where the config file should really be
  r = parse_config("/home/rickse2/Documents/csci49X/secure-port-reservation/sprd.conf");
  // Step through list of reservations, binding to each port and storing the socket fd
  // TODO: see if there's a nice way to reduce amount of duplicate code in this loop
  // TODO: consider modifying sockList to additionally contain fields defining user and group permissions
  memset(&sockList, 0, sizeof(sockList));
  struct sockaddr_in sockInfo;
  while (r != NULL) {
    range_node* ports = r->port_head;
    while (ports != NULL) {
        // Bind to first port in range
        int portNum = ports->range[0];
        // TODO: double check that this range makes sense
        if (portNum > 0 && portNum < NUM_PORTS) {
            memset(&sockInfo, 0, sizeof(sockInfo));
            sockInfo.sin_family = AF_INET;
            sockInfo.sin_port = htons(portNum);
            if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                // call to socket() failed
                syslog(LOG_ALERT, "Unable to acquire a file descriptor for port %d", portNum);
                syslog(LOG_ALERT, "Unable to reserve port %d", portNum);
            } else if (bind(fd, (struct sockaddr *)&sockInfo, sizeof(sockInfo)) < 0) {
                // call to bind() failed
                syslog(LOG_ALERT, "Unable to reserve port %d", portNum);
            } else {
                // successful bind - store socket in sockList
                sockList[portNum][1] = fd;
            }
        } else {
            // Not a valid port number
            syslog(LOG_WARNING, "Invalid port number: %d", portNum);
        }
        for (i = portNum+1; i <= ports->range[1]; i++) {
            portNum = i;
            if (portNum > 0 && portNum < NUM_PORTS) {
                memset(&sockInfo, 0, sizeof(sockInfo));
                sockInfo.sin_family = AF_INET;
                sockInfo.sin_port = htons(portNum);
                if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    // call to socket() failed
                    syslog(LOG_ALERT, "Unable to acquire a file descriptor for port %d", portNum);
                    syslog(LOG_ALERT, "Unable to reserve port %d", portNum);
                } else if (bind(fd, (struct sockaddr *)&sockInfo, sizeof(sockInfo)) < 0) {
                    // call to bind() failed
                    syslog(LOG_ALERT, "Unable to reserve port %d", portNum);
                } else {
                    // successful bind - store socket in sockList
                    sockList[portNum][1] = fd;
                }
            } else {
                // Not a valid port number
                syslog(LOG_WARNING, "Invalid port number: %d", portNum);
            }
        }
        ports = ports->next;
    }
    r = r->next;
  }

  // Unlink named fifo
  if (unlink(NAMED_FIFO) < 0) {
    if (errno != ENOENT) {
        syslog(LOG_NOTICE, "Unable to unlink the named fifo '%s'", NAMED_FIFO);
    }
  }

  // Use chmod(2) for:
  // WRITE: S_IWUSR (for current testing only), S_IWOTH and S_IWGRP
  // READ: S_IRUSR
  // TODO: Change to O_RDONLY after testing
  if((namedFifo = mkfifo(NAMED_FIFO, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH)) < 0) {
    syslog(LOG_CRIT, "Failed to make the named fifo '%s'", NAMED_FIFO);
    exit(EXIT_FAILURE);
  }

  while(1) {
    // TODO: Get uds socket then FD_SET the uds socket into the active_fset
    read_fdset = active_fdset;
    if (select (FD_SETSIZE, &read_fdset, NULL, NULL, NULL) == 0) {
      for (i = 0; i < FD_SETSIZE; i++) {
	    if (FD_ISSET(i, &read_fdset)) {
          if (i == namedFifo) {
	        /* run secure_bind interface, read uds name from named fifo, try to connect to that uds */
	        handleNewConnection(namedFifo, &active_fdset);
	        /* This is the case where it is not the named fifo, so it is one of possibly many ud sockets */
	        /* Need to check what was sent across, if 0, shutdown connection and FD_CLR, if not 0, then they have sent credentials/port request */
          } else {
	        //fprintf(stderr, "SELECT: %d", uds); //test Ray -- can we delete this?
	        memset(portBuf, 0, sizeof(portBuf));
	        if ((recv(i, portBuf, PORT_DIGIT_MAX, MSG_PEEK)) < 0) {
              syslog(LOG_ERR, "Failed to recieve data from file descriptor %d", i);
	          break;
	        } else if (strlen(portBuf) == 0) {
	          /* This is the case of secure_close, i can be shutdown and FD_CLR'ed from active_fdset */
                if (shutdown(i, SHUT_RDWR) < 0) {
                    syslog(LOG_CRIT, "Failed to shut down the file descriptor %d", i);
                }

                FD_CLR(i, &active_fdset);
	        } else {
	          /* This is the case of secure_bind */
	          if((handleExistingConnection(i, portBuf)) < 0) {
		    FD_CLR(i, &active_fdset);
		    if((close(i)) < 0) {
		      syslog(LOG_CRIT, "Failed to shut down the file descriptor %d", i);
		    }
		  }
	        }

          }
	    }
	  }
    } else {
        syslog(LOG_INFO, "Failed to select a file descriptor from the read_fdset");
    }
  }
}

/* Function to handle a request on a unix domain socket, return 0 on success, -1 on failure */

int handleExistingConnection (int uds, char *portBuf) {

  struct msghdr credMsg, fdMsg;
  struct cmsghdr *passCred, *cmsg;
  /* Begin BSD incompatible */
  char cDataBuf[CMSG_SPACE(sizeof(struct ucred))], cmsgbuf[CMSG_SPACE(sizeof(int))];
  struct ucred *realCred;
  /* End BSD incompatible */
  struct iovec inData, tempData;
  int passFDData = 1;
  int flag = 1;
  int fd = 0;

  memset(&credMsg, 0, sizeof(credMsg));
  memset(cDataBuf, 0, sizeof(cDataBuf));
  memset(&inData, 0, sizeof(inData));
  memset(portBuf, 0, PORT_DIGIT_MAX + 1);

  credMsg.msg_control = cDataBuf;
  credMsg.msg_controllen = sizeof(cDataBuf);
  credMsg.msg_iov = &inData;
  credMsg.msg_iovlen = 1; //may need to be changed from 1 to sizeof(portBuf)
  inData.iov_base = portBuf;
  inData.iov_len = sizeof(portBuf);

  if((setsockopt(uds, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0){
    syslog(LOG_ERR, "Could not set SO_PASSCRED on unix domain socket options");
    return RETURN_FAILURE;
  }

  if((recvmsg(uds, &credMsg, 0)) < 0){
    syslog(LOG_ERR, "Could not receive message from requester");
    return RETURN_FAILURE;
  }

  passCred = CMSG_FIRSTHDR(&credMsg);

  if(passCred == NULL || passCred->cmsg_type != SCM_CREDENTIALS){
    syslog(LOG_WARNING, "Did not receive credentials from requester");
    return RETURN_FAILURE;
  }

  realCred = (struct ucred *)CMSG_DATA(passCred);

  flag = 0;

  if((setsockopt(uds, SOL_SOCKET, SO_PASSCRED, &flag, sizeof(flag))) < 0){
    syslog(LOG_WARNING, "Could not take SO_PASSCRED off unix domain socket options");
    return RETURN_FAILURE;
  }

  //TODO: check data structure add in if statements below

  /* Credentials match and port open */

  memset(&fdMsg, 0, sizeof(fdMsg));
  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  fdMsg.msg_control = cmsgbuf;
  fdMsg.msg_controllen = sizeof(cmsgbuf);
  cmsg = CMSG_FIRSTHDR(&fdMsg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));
  memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));
  fdMsg.msg_controllen = cmsg->cmsg_len;
  fdMsg.msg_iov = &tempData;
  fdMsg.msg_iovlen = 1;
  tempData.iov_base = &passFDData;
  tempData.iov_len = sizeof(int);

  if((sendmsg(uds, &fdMsg, 0)) < 0) {
    syslog(LOG_CRIT, "Could not send socket");
    return RETURN_FAILURE;
  }

  syslog(LOG_INFO, "Passed socket with port %d to uid: %ld gid: %ld", atoi(portBuf), (long)realCred->uid, (long)realCred->gid);

  /* Credentials match and port not open */

  syslog(LOG_INFO, "Port %d already in use", atoi(portBuf));
  return RETURN_FAILURE;

  /* Credentials do not match */

  syslog(LOG_NOTICE, "Port %d not authorized for user uid: %ld gid: %ld", atoi(portBuf), (long)realCred->uid, (long)realCred->gid);
  return RETURN_FAILURE;


  return RETURN_SUCCESS;

}

/* Function to handle a request on the named FIFO, returns 0 on success, -1 on failure */

int handleNewConnection (int namedFifo, fd_set *active_fdset) {

  char readBuf[PATH_MAX + 1];
  char sendChar = 'd';
  struct sockaddr_un remote;
  int len, connectSock;

  memset(readBuf, 0, sizeof(readBuf));
  memset(&remote, 0, sizeof(remote));

  if((read(namedFifo, readBuf, PATH_MAX)) == 0) {
    syslog(LOG_NOTICE, "No data read from named FIFO");
    return RETURN_FAILURE;
  }

  if((connectSock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_CRIT, "Could not open a unix domain socket");
    return RETURN_FAILURE;
  }
  remote.sun_family = AF_UNIX;
  strncpy(remote.sun_path, readBuf, strlen(readBuf));
  len = sizeof(remote);

  if((connect(connectSock, (struct sockaddr *)&remote, len)) < 0) {
    syslog(LOG_WARNING, "Could not connect to requested unix domain socket");
    return RETURN_FAILURE;
  }

  if((send(connectSock, &sendChar, sizeof(sendChar), 0)) < 0) {
    syslog(LOG_ERR, "Could not send credentials across unix domain socket");
    return RETURN_FAILURE;
  }

  /* Add to active_fdset to monitor */
  FD_SET(connectSock, active_fdset);

  return RETURN_SUCCESS;
}
