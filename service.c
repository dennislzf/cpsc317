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

int commandBufferCapcity = 10;
int commandBufferContent = 0;
int isSocketClosed = 0;

char* requestparse[7];

void handle_client(int socket) {
    char *commandBuffer = malloc(sizeof(char) * commandBufferCapcity);

    while(1){
        if(isSocketClosed){
            printf("Socket closed. Connection end");
            return;
        }

        receive(socket,commandBuffer);
        //parseCommand(commandBuffer, commandBufferContent);
        //buildRespone();
        //sendResponse():
        flushCommandBuffer(commandBuffer);

        isSocketClosed = 1;
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
        if(receivedBytes == -1){
            printf("ERROR");
        }
        else{
            printf("Received %d bytes\n", receivedBytes);
        }
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
    while(receivedBytes > 2 && commandBuffer[currentBufferCopyPos-1] == 13 && commandBuffer[currentBufferCopyPos-2] == 10); // while there is data to read

    printf("Done receiving");
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
    //free(commandBuffer);

    return newCommandBuffer;
}
/*
 * Resets the position of the command buffer content to 0
 * so we can reuse it
 */
void flushCommandBuffer(char* commandBuffer){
    commandBuffer[0] = '\0';
    commandBufferContent = 0;
}
