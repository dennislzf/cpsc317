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

int commandBufferCapcity = 100;
int commandBufferContent = 0;

void handle_client(int socket) {
    char *commandBuffer = malloc(sizeof(char) * commandBufferCapcity);


    while(1){
        receive(socket,commandBuffer);
        //parseCommand();
        //buildResponse();
        //sendResponse();
        flushCommandBuffer();
        printf("\nDone\n");
        break;
    }
    return;
}

/*
 * Receives a command from a socket
 * and writes it into the commandBuffer
 *
 * We fist read it into a small local buffer and then copy it over
 * If the command buffer is too small, we are resizing
 */
void receive(int socket,char* commandBuffer){
    int receiveBufferSize = 100;
    int currentBufferCopyPos = 0;
    char receive_buffer[receiveBufferSize];

    int receivedBytes;
    do{
        // Read 'receiveBufferSize' chars into the buffer
        receivedBytes = recv(socket, receive_buffer, receiveBufferSize,0);


        // Check if we need to resize the command array
        if((receivedBytes + currentBufferCopyPos) >= commandBufferCapcity){
            printf("Resizing\n");
            commandBuffer = resizeCommandArray(commandBuffer);
        }

        // Copy the received bytes into the command buffer
        int i;
        for(i = 0; i < receivedBytes;i++){
            commandBuffer[currentBufferCopyPos] = receive_buffer[i];
            currentBufferCopyPos += 1;
        }

        commandBufferContent = currentBufferCopyPos;
    }
    while(receivedBytes > 0); // while there is data to read


}
/*
 * This resizes the command array and copies the old elements over
 */
char * resizeCommandArray(char* commandBuffer){
    // Create a new buffer
    commandBufferCapcity = commandBufferCapcity * 2;
    char *newCommandBuffer = malloc(sizeof(char) * commandBufferCapcity);

    // Copy the old elements over
    int index;
    for(index = 0; index < commandBufferContent;index++){
        newCommandBuffer[index] = commandBuffer[index];
    }

    // Delete the old command Buffer
    free(commandBuffer);

    return newCommandBuffer;
}
/*
 * Resets the position of the command buffer content to 0
 * so we can reuse it
 */
void flushCommandBuffer(){
    commandBufferContent = 0;
}
