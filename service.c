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
            printf("Socket closed. Connection end\n");
            return;
        }

        receive(socket,commandBuffer);
        if(requestIsValid(commandBuffer,commandBufferContent)){
            parseCommand(commandBuffer, commandBufferContent);
            //buildRespone();
            //sendResponse():
        }
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
            resizeCommandArray(commandBuffer);
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

    printf("Done receiving\n");
    printBuffer(commandBuffer,commandBufferContent);
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
    for(index = 0; index <= commandBufferContent;index++){
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

char* parseCommand(char* buffer, int buffersize){



    char*  querystring[4];
    char* requesttype;
    char* commandtype;
    int querystringval = 0;
    int startofcommand;
    int parameter = 0;
    //if request type is GET;
    if(buffer[0] == 'G'){
        requesttype = "GET";
        //if request type is POST
    }else if (buffer[0] == 'P'){
        requesttype = "POST";
    }
    //if it is a post, command starts at 5th bucket
    if(strcmp(requesttype,"POST") == 0){
        startofcommand = 5;
    }
    //if request type is a get, command starts at 4th bucket
    else if (strcmp(requesttype, "GET") == 0){
        startofcommand = 4;
    }
    //if after request, browser puts url, direct the start of command to the beginning of request type
    if(buffer[startofcommand] == 'h'){
        int i ;
        for( i = startofcommand + 8; i < buffersize ; i++){
            if(buffer[i] == '/'){
                startofcommand = i;
            }
        }
    }

    //get command type of request.
    switch(buffer[startofcommand +1]){

    case('l'):
        if (buffer[startofcommand + 4] == 'i')
            commandtype = "login";
        else commandtype = "logout";
        break;
    case ('s'):
        commandtype = "servertime";
        break;
    case('b'):
        commandtype = "browser";
        break;
    case('r'):
        commandtype = "redirect";
        break;
    case('g'):
        commandtype = "getfile";
        break;
    case('a'):
        commandtype = "addcart";
        break;
    case('d'):
        commandtype = "delcart";
        break;
    case('c'):
        if(buffer[startofcommand + 2] == 'h'){
            commandtype = "checkout";
        }
        else commandtype = "close";
        break;
    default:
        commandtype = "Command not found.";
        break;
    }
    // check if there is a query string.
    int o;
    for( o = 0; o < buffersize; o++)
    {
        if(buffer[o] == '?')
            querystringval = o;
    }
    // if there are parameters in the query string, parse it and place into bucket.
    if(querystringval != 0){
        int startofstring = querystringval +1;
        //number of parameters

        //start of parameter char array
        int parameterstring = 0 ;
        // if there are more parameters, value to 1;
        int moreparameter = 1;
        int p;
        while(moreparameter == 1){
            for( p = startofstring; p < buffersize ; p++){
                if(buffer[p] == '&'){
                    startofstring = p+1;
                    break;
                }
                if(buffer[p] == ' '){
                    moreparameter = 0;
                    break;
                }

                querystring[parameter][parameterstring] = buffer[p];
                parameterstring++;

            }
            parameter++;

        }
    }

    //return the string array containing all the values;
    requestparse[0] = requesttype;
    requestparse[1] = commandtype;
    int numparameter = 0;
    int q;
    for(q  = 2; q < 2 + parameter; q++){
        requestparse[q] = querystring[numparameter];
        numparameter++;
    }
    printf("hihi");
    printf("REQUESTTYPE:%s\n",requestparse[0]);
    printf("COMMANDTYPE:%s\n",requestparse[1]);
    printf("QUERYSTRING1%s\n",requestparse[2]);
    printf("QUERYSTRING2%s\n",requestparse[3]);

    return *requestparse;
}
/*
 * Returns 1 if we have a GET or POST at the beginning of the message
 * 0 otherwise
 */
int requestIsValid(char *buffer, int buffersize){
    // Check if we have space for a "GET"
    if(buffersize >= 3){
        if(buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T'){
            return 1;
        }
    }
    if(buffersize >= 4){
        if(buffer[0] == 'P' && buffer[1] == 'O' && buffer[2] == 'S' && buffer[3] == 'T'){
            return 1;
        }
    }
    return 0;
}
/*
 * Print buffer
 */
void printBuffer(char *buffer, int buffersize){
    printf("Buffer content: ");
    int i;
    for(i = 0;i <= buffersize;i++){
        printf("%c",buffer[i]);
    }
    printf("\n");
}
