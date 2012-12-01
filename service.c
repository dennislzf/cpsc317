/*
 * File: service.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "service.h"
#include "util.h"

char receive_buffer[3000];

void handle_client(int socket) {

    /* TODO Loop receiving requests and sending appropriate responses,
     *      until one of the conditions to close the connection is
     *      met.
     */
    while(1){
        //receive();
        //parseCommand();
        //buildResponse();
        //sendResponse();
    }
    return;
}
