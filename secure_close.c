#define _GNU_SOURCE
#include "spr.h"
#include <unistd.h>

int secure_close (int udsSock, int tcpSock) {
    if ((close(udsSock)) < 0) { 
        return -1;
    }

    return close(tcpSock);
}
