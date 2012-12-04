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
#include <ctype.h>

#include "service.h"
#include "util.h"

int commandBufferCapcity = 1000;
int commandBufferContent = 0;
int isSocketClosed = 0;
int clientSocket =-1;
char* requestparse[7];

void handle_client(int socket) {
    char *commandBuffer = malloc(sizeof(char) * commandBufferCapcity);
    clientSocket = socket;
    while(1){
        if(isSocketClosed){
            printf("Socket closed. Connection end");
            return;
        }

        receive(socket,commandBuffer);
        printBuffer(commandBuffer,commandBufferContent);
        if(requestIsValid(commandBuffer,commandBufferContent)){
            parseCommand(commandBuffer, commandBufferContent);
        }
        else{
            printf("Invalid Syntax\n");
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
    while(receivedBytes > 2 && commandBuffer[currentBufferCopyPos-2] != 13 && commandBuffer[currentBufferCopyPos-1] != 10); // while there is data to read

    printf("Done receiving");
}


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
    querystring[1] =malloc(99);
    querystring[2] =malloc(99);
    querystring[3] =malloc(99);
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
                break;
            }
        }
    }

    int i = 0;
    int ii = 0;
    char strservertime[10];


    //get command type of request.
    switch( tolower(buffer[startofcommand +1])){

    case('l'):
        if (buffer[startofcommand + 4] == 'i'){
            commandtype = "login";
            if(buffer[startofcommand + 6] != ' ' && buffer[startofcommand + 6] != '?'){
                handleLogin(404);
            }
        }
        else{
            commandtype = "logout";
        }
        break;

    case ('s'):
        if(buffer[startofcommand + 11] != ' ' && buffer[startofcommand + 11] != '?'){
            handleServerTimeRequest(404);
        }
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strservertime[ii] = tolower(buffer[i]);
            ii++;
        }
        if(strcmp(strservertime,"servertime") == 0){
            handleServerTimeRequest(200);
        }else handleServerTimeRequest(404);

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
        if(buffer[o] == '?'){
            querystringval = o;
            break;
        }
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
            parameterstring= 0;

            for( p = startofstring; p < buffersize ; p++){
                if(buffer[p] == '&'){
                    startofstring = p+1;
                    moreparameter = 1;
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



    return *requestparse;
}


/*
 * Print buffer helper function
 */
void printBuffer(char *buffer, int buffersize){
    printf("\nBuffer content: ");
    int i;
    for(i = 0;i <= buffersize;i++){
        printf("%c",buffer[i]);
    }
    printf("\n");
}
void handleLogin(int i){
}
/*
 * Handles a servertime request from the client
 */
void handleServerTimeRequest(int statusCode){
    printf("The status code is %d",statusCode);
    char* deliveryCodeMessage = getDeliveryCode(statusCode);
    int deliveryCodeMessageLength = getArraySize(deliveryCodeMessage);

    sendToClient(deliveryCodeMessage,deliveryCodeMessageLength);
}
/*
 * Returns a message corresponding to the delivery code and its length
 */
char* getDeliveryCode(int statusCode){
    char *deliveryCodeMessage;

    if(statusCode == 200){
        deliveryCodeMessage = "HTTP/1.1 200 OK";
    }
    else{
        deliveryCodeMessage = "HTTP/1.1 404 not found";
    }
    return deliveryCodeMessage;
}
/*
 * Gets the size of an array
 */
int getArraySize(char* array){
    int count = 0;
    int i = 0;
    while(array[i] != '\0'){
        count++;
        i++;
    }
    return count;
}
/*
 * Sends a char array over the globally defined socket
 */

void sendToClient(char* content, int contentLength){
    //int bytesToSend = contentLength;
    send(clientSocket,content,contentLength,0);
}

/*
 * Checks whether we even have a GET or POST request
 * returns 1 if yes, 0 if not
 */

int requestIsValid(char *buffer, int buffersize){
    if(buffersize >= 3){
        if(buffer[0]=='G' && buffer[1]=='E' && buffer[2]=='T'){
            return 1;
        }
    }
    if(buffersize >= 4){
        if(buffer[0]=='P' && buffer[1]=='O' && buffer[2]=='S' && buffer[3]=='T'){
            return 1;
        }
    }
    return 0;
}
