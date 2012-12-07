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
#include <time.h>

#include "service.h"
#include "util.h"

int commandBufferCapcity = 1000;
int commandBufferContent = 0;
int isSocketClosed = 0;
int clientSocket =-1;
int isLoggedIn = 0;
char *userAgent;
char *loggedInUserName;
char* requestparse[7];

void handle_client(int socket) {
    char *commandBuffer = malloc(sizeof(char) * commandBufferCapcity);
    clientSocket = socket;
    while(1){
        if(isSocketClosed){
            printf("Socket closed. Connection end\n");
            return;
        }

        receive(socket,commandBuffer);
        if(requestIsValid(commandBuffer,commandBufferContent)){
            extractUserAgent(commandBuffer);
            parseCommand(commandBuffer, commandBufferContent);
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
    // while there is data to read
    while(receivedBytes > 2 && commandBuffer[currentBufferCopyPos-2] != 13 && commandBuffer[currentBufferCopyPos-1] != 10);
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

void parseCommand(char* buffer, int buffersize){

    char* requesttype;


    int startofcommand;
    
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
    char* strservertime = malloc(99);


    //get command type of request.
    switch( tolower(buffer[startofcommand +1])){

    case('l'):
        if (buffer[startofcommand + 4] == 'i'){
            parseLogin(buffer,startofcommand,buffersize);
        }
        else{
            parseLogout(buffer,startofcommand,buffersize);
        }
        break;

    case ('s'):
        if(buffer[startofcommand + 11] != ' ' && buffer[startofcommand + 11] != '?'){

            throw404();
        }
        else{
            for (i = startofcommand + 1; i <startofcommand + 11;i++){
                strservertime[ii] = tolower(buffer[i]);
                ii++;
            }

            if(strncmp(strservertime,"servertime",10) == 0){
                handleServerTimeRequest();
            }else {

                throw404();
            }
        }

        break;
    case('b'):
        parseBrowser(buffer,startofcommand,buffersize);
        break;
    case('r'):
        parseRedirect(buffer,startofcommand,buffersize);
        break;
    case('g'):
        parseGetFile(buffer,startofcommand,buffersize);
        break;
    case('a'):
        parseAddCart(buffer,startofcommand,buffersize);
        break;
    case('d'):
        parseDelCart(buffer,startofcommand,buffersize);
        break;
    case('c'):
        if(buffer[startofcommand + 2] == 'h'){
            parseCheckout(buffer,startofcommand,buffersize);
        }
        else{
            parseClose(buffer,startofcommand,buffersize);
        }
        break;
    default:
        throw404();
        break;
    }
}

void parsePutFile(char* buffer,int startofcommand,int buffersize){
    char* querystring = malloc(999);
    char* putfilestring = malloc(99);
    char* contentstring = malloc(999);
    char* strcheck = malloc(99);
    int i = 0;
    int ii = 0;
    int k = 0;
    int kk = 0;
    int putfilelength = 0;
    int contentstringlength =0;
    if(strstr(buffer,"?filename=") != NULL || strstr(buffer,"&filename=") != NULL){
        querystring = strstr(buffer,"filename=");

        for(i = 10; i <99; i ++){
            if(querystring[i] == ' ' || querystring[i] == '&'){
                break;
            }

            putfilestring[k] = querystring[i];
            k++;
        }

    }else{

        throw404();
        return;
    }

    if(strstr(buffer,"?content=") != NULL || strstr(buffer,"&content=") != NULL){
        querystring = strstr(buffer,"content=");

        for(i = 8; i <999; i ++){
            if(querystring[i] == ' ' || querystring[i] == '&'){
                break;
            }

            contentstring[kk] = querystring[i];
            kk++;
        }

    }else{

        throw404();
        return;
    }


    //check if client types login
    if(buffer[startofcommand + 8] != ' ' && buffer[startofcommand + 8] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"putfile",7) == 0){
            putfilestring[k] = '\0';
            contentstring[kk] = '\0';
            contentstringlength = kk;
            putfilelength = k;
            handlePutFileRequest(putfilestring,contentstring,putfilelength,contentstringlength);
        }else {

            throw404();

        }
    }

}

void parseLogout(char* buffer, int startofcommand,int buffersize){
    char* strcheck= malloc(99);
    int i;
    int ii = 0;
    //check if client types login
    if(buffer[startofcommand + 7] != ' ' && buffer[startofcommand + 7] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"logout",6) == 0){

            handleLogoutRequest();
        }else {

            throw404();

        }
    }

}
void parseLogin(char* buffer, int startofcommand,int buffersize){
    char* querystring = malloc(99);
    char* usernamestring = malloc(99);
    char* strcheck = malloc(99);
    int i = 0;
    int ii = 0;
    int k = 0;
    int usernamelength = 0;
    if(strstr(buffer,"?username=") != NULL || strstr(buffer,"&username=") != NULL){
        querystring = strstr(buffer,"username=");
        for(i = 9; i < 99; i ++){
            if(querystring[i] == ' ' || querystring[i] == '&' || querystring[i] == 0){
                break;
            }
            usernamestring[k] = querystring[i];
            usernamestring[++k] = '\0';

        }

    }else{
        throw404();
        return;
    }

    //check if client types login
    if(buffer[startofcommand + 6] != ' ' && buffer[startofcommand + 6] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        // Actually login
        if(strncmp(strcheck,"login",5) == 0){
            usernamelength = strlen(usernamestring);
            handleLoginRequest(usernamestring,usernamelength);
        }else {
            throw404();
        }
    }

}
void parseCheckout(char* buffer, int startofcommand,int buffersize ) {

    char* strcheck= malloc(99);
    int i;
    int ii = 0;
    //check if client types login
    if(buffer[startofcommand + 9] != ' ' && buffer[startofcommand + 9] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"checkout",8) == 0){

            handleCheckoutRequest();
        }else {

            throw404();

        }
    }

}
void parseBrowser(char* buffer, int startofcommand,int buffersize){

    char* strcheck = malloc(99);
    int i;
    int ii = 0;
    //check if client types login
    if(buffer[startofcommand + 8] != ' ' && buffer[startofcommand + 8] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"browser",7) == 0){

            handleBrowserRequest();
        }else {

            throw404();

        }
    }

}
void parseRedirect(char* buffer, int startofcommand,int buffersize){
    char* querystring = malloc(99);
    char* redirectstring = malloc(99);
    char* strcheck = malloc(99);
    int i = 0;
    int ii = 0;
    int k = 0;
    int redirectlength = 0;
    if(strstr(buffer,"?redirect=") != NULL || strstr(buffer,"&redirect=") != NULL){
        querystring = strstr(buffer,"redirect=");

        for(i = 9; i <99; i ++){
            if(querystring[i] == ' ' || querystring[i] == '&' ){
                break;
            }

            redirectstring[k] = querystring[i];
            k++;
        }

    }else{

        throw404();
        return;
    }



    //check if client types login
    if(buffer[startofcommand + 9] != ' ' && buffer[startofcommand + 9] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"redirect",8) == 0){
            redirectlength = k;
            redirectstring[k] = '\0';
            handleRedirectRequest(redirectstring,redirectlength);
        }else {

            throw404();

        }
    }

}
void parseGetFile(char* buffer, int startofcommand,int buffersize){
    char* querystring = malloc(99);
    char* filenamestring = malloc(99);
    char* strcheck = malloc(99);
    int i = 0;
    int ii = 0;
    int k = 0;
    int filenamelength = 0;
    if(strstr(buffer,"?filename=") != NULL || strstr(buffer,"&filename=") != NULL){
        querystring = strstr(buffer,"filename=");

        for(i = 9; i <99; i ++){
            if(querystring[i] == ' ' || querystring[i] == '&' ){
                break;
            }

            filenamestring[k] = querystring[i];
            k++;
        }

    }else{

        throw404();
        return;
    }



    //check if client types login
    if(buffer[startofcommand + 8] != ' ' && buffer[startofcommand + 8] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"getfile",8) == 0){
            filenamelength = k;
            filenamestring[k] = '\0';
            handleGetFileRequest(filenamestring,filenamelength);
        }else {

            throw404();

        }
    }

}
void parseAddCart(char* buffer, int startofcommand,int buffersize){
    char* querystring = malloc(99);
    char* addcartstring = malloc(99);
    char* strcheck = malloc(99);
    int i = 0;
    int ii = 0;
    int k = 0;
    int addcartlength = 0;
    if(strstr(buffer,"?item=") != NULL || strstr(buffer,"&item=") != NULL){
        querystring = strstr(buffer,"item=");

        for(i = 5; i <99; i ++){
            if(querystring[i] == ' ' || querystring[i] == '&' ){
                break;
            }

            addcartstring[k] = querystring[i];
            k++;
        }

    }else{

        throw404();
        return;
    }



    //check if client types login
    if(buffer[startofcommand + 8] != ' ' && buffer[startofcommand + 8] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"addcart",8) == 0){

            addcartstring[k] = '\0';
            addcartlength = k;
            handleAddCartRequest(addcartstring,addcartlength);
        }else {

            throw404();

        }
    }

}
void parseDelCart(char* buffer, int startofcommand,int buffersize){
    char* querystring = malloc(99);
    char* delstring = malloc(99);
    char* strcheck = malloc(99);
    int i = 0;
    int ii = 0;
    int k = 0;

    if(strstr(buffer,"?itemnr=") != NULL || strstr(buffer,"&itemnr=") != NULL){
        querystring = strstr(buffer,"itemnr=");

        for(i = 7; i <99; i ++){
            if(querystring[i] == ' ' || querystring[i] == '&' ){
                break;
            }

            delstring[k] = querystring[i];
            k++;
        }

    }else{

        throw404();
        return;
    }



    //check if client types login
    if(buffer[startofcommand + 8] != ' ' && buffer[startofcommand + 8] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"delcart",7) == 0){
            delstring[k] = '\0';

            handleDelCartRequest(delstring);
        }else {

            throw404();

        }
    }
}

void parseClose(char* buffer, int startofcommand,int buffersize) {
    char* strcheck= malloc(99);;
    int i;
    int ii = 0;
    //check if client types login
    if(buffer[startofcommand + 6] != ' ' && buffer[startofcommand + 6] != '?'){
        throw404();
    }else{
        for (i = startofcommand + 1; i <startofcommand + 11;i++){
            strcheck[ii] = tolower(buffer[i]);
            ii++;
        }

        if(strncmp(strcheck,"close",5) == 0){

            handleCloseRequest();
        }else {

            throw404();

        }
    }

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
/*
 * Logs in the client with the username and sets some local variables
 */
void handleLoginRequest(char* querystring, int querystringsize){
    // First do some cleanup. This is a bug, fix it later if there is time
    char *username = malloc(querystringsize);
    strncpy(username,querystring,querystringsize);
    username[querystringsize] = '\0';

    // Set the local user to logged in
    loggedInUserName = username;
    isLoggedIn = 1;
    // Get the various header messages

    // The delivery code message
    char* deliveryCodeMessage = getDeliveryCode(200);
    int deliveryLength = strlen(deliveryCodeMessage);

    // The connection message
    char* connectionMessage = getConnectionMessage(1); // keep the connection open
    int connectionLength = strlen(connectionMessage);

    // The date message
    char* dateMessage = getGMTDateMessage();
    int dateLength = strlen(dateMessage);

    // Create cookie content
    char * directive = "username=";
    char *cookieContent = malloc(strlen(username) + strlen(directive));
    strcpy(cookieContent,directive);
    strcat(cookieContent,username);

    // Set the cookie
    char* cookieMessage = setCookie(cookieContent);
    int cookieLength = strlen(cookieMessage);

    // Content message
    char * contentMessage = querystring;
    int contentLength = strlen(contentMessage);

    // Copy together all the strings
    char* finalString;
    finalString = malloc(deliveryLength + connectionLength + dateLength  + contentLength + cookieLength);
    strcpy(finalString, deliveryCodeMessage);
    strcat(finalString, connectionMessage);
    strcat(finalString, dateMessage);
    strcat(finalString, cookieMessage);
    strcat(finalString, contentMessage);

    sendToClient(finalString,strlen(finalString));

    //free memory
    free(finalString);
    free(cookieContent);
}
/*
 * Handles a servertime request from the client
 */
void handleServerTimeRequest(){
    printf("Server time\n");
    // The delivery code message
    char* deliveryCodeMessage = getDeliveryCode(200);
    int deliveryLength = strlen(deliveryCodeMessage);

    // The connection message
    char* connectionMessage = getConnectionMessage(1); // keep alive
    int connectionLength = strlen(connectionMessage);

    // The date message
    char* dateMessage = getGMTDateMessage();
    int dateLength = strlen(dateMessage);
    // Content message
    char *contentMessage = getLocalDateMessage(); // get the current local server time
    int contentLength = strlen(contentMessage); // need to prepend a newline

    // The cache message
    char* cacheMessage = getCacheMessage(0); // no cache
    int cacheLength = strlen(cacheMessage);

    // Copy together all the strings
    char* finalString;
    finalString = malloc(deliveryLength + connectionLength + dateLength + cacheLength + contentLength);
    strcpy(finalString, deliveryCodeMessage);
    strcat(finalString, connectionMessage);
    strcat(finalString, dateMessage);
    strcat(finalString, cacheMessage);
    strcat(finalString, contentMessage);

    sendToClient(finalString,strlen(finalString));

    // Free memory
    free(finalString);
}
/*
 * Returns a message corresponding to the delivery code and its length
 */
char* getDeliveryCode(int statusCode){
    char *deliveryCodeMessage;

    if(statusCode == 200){
        deliveryCodeMessage = "HTTP/1.1 200 OK\n";
    }
    else{
        deliveryCodeMessage = "HTTP/1.1 404 not found\n";
    }
    return deliveryCodeMessage;
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
/*
 * Gets the connection message string
 * 0 for close
 */
char* getConnectionMessage(int command){
    char *connectionMessage;

    if(command == 1){
        connectionMessage = "Connection: keep alive\n";
    }
    else{
        connectionMessage = "Connection: close\n";
    }
    return connectionMessage;
}
/*
 * Gets the cache control setting string
 * 0 - no cache
 * 1 - private
 * 2 - public
*/
char* getCacheMessage(int command){
    char *cacheMessage;

    if(command == 0){
        cacheMessage = "Cache-Control: no-cache\n";
    }
    else if(command == 1){
        cacheMessage = "Cache-Control: private\n";
    }
    else{
        cacheMessage = "Cache-Control: public\n";
    }
    return cacheMessage;

}

/*
 * Gets the current date message string in GMT
 */
char* getGMTDateMessage(){
    char *dateMessage = malloc(sizeof(char) * 256);

    time_t timeData;
    struct tm * currentTime;
    time(&timeData);
    currentTime = gmtime(&timeData);
    strftime(dateMessage,256,"%a, %d %b %Y %H:%M:%S\n",currentTime);

    return dateMessage;
}


/*
 * Gets the current date message string in GMT
 */
char* getLocalDateMessage(){
    char *dateMessage = malloc(sizeof(char) * 256);

    time_t timeData;
    struct tm * currentTime;
    time(&timeData);
    currentTime = localtime(&timeData);
    strftime(dateMessage,256,"\n%a, %d %b %Y %H:%M:%S\n",currentTime);

    return dateMessage;
}

void handleLogoutRequest(){
    printf("logout");
}
char* getUserAgent(){
    return userAgent;
}

/*
 * Sends a 404 back to the client
 */
void throw404(){
    printf("404 error\n");
    // The delivery code message
    char* deliveryCodeMessage = getDeliveryCode(404);
    int deliveryLength = strlen(deliveryCodeMessage);

    // The connection message
    char* connectionMessage = getConnectionMessage(0); // close the connection
    int connectionLength = strlen(connectionMessage);

    // The date message
    char* dateMessage = getGMTDateMessage();
    int dateLength = strlen(dateMessage);

    // Content message
    char * contentMessage = "\nCommand not found\n";
    int contentLength = strlen(contentMessage);

    // The cache message
    char* cacheMessage = getCacheMessage(0);
    int cacheLength = strlen(cacheMessage);

    // Copy together all the strings
    char* finalString;
    finalString = malloc(deliveryLength + connectionLength + dateLength + cacheLength + contentLength);
    strcpy(finalString, deliveryCodeMessage);
    strcat(finalString, connectionMessage);
    strcat(finalString, dateMessage);
    strcat(finalString, cacheMessage);
    strcat(finalString, contentMessage);

    sendToClient(finalString,strlen(finalString));

    // Free memory
    free(finalString);


}
/**
 * Extracts the userAgent from the header
 */
void extractUserAgent(char *buffer){
    char *substring = strstr(buffer,"User-Agent:");

    // No User Agent provided
    if(substring == NULL){
        return;
    }
    userAgent = strtok(substring,"\n");
    memmove(userAgent,userAgent + 12,strlen(userAgent)); // remove the first 12 characters from the string

}
/**
 * Returns a string with the Set Cookie directive
 */
char* setCookie(char *msg){
    char *partOne = "Set-Cookie: ";
    char *partTwo = "; Max-Age=86400; Version=1";
    char* finalString;
    finalString = malloc(strlen(msg) + strlen(partOne)+ strlen(partTwo));
    strcpy(finalString, partOne);
    strcat(finalString, msg);
    strcat(finalString, partTwo);

    return finalString;

}

/**
 * Displays the user agent to the user
 */
void handleBrowserRequest(){
    printf("Handling Browser\n");
    char* deliveryCodeMessage = getDeliveryCode(200);
    int deliveryLength = strlen(deliveryCodeMessage);

    // The connection message
    char* connectionMessage = getConnectionMessage(1); // keep alive
    int connectionLength = strlen(connectionMessage);

    // The date message
    char* dateMessage = getGMTDateMessage();
    int dateLength = strlen(dateMessage);

    // Content message
    char * contentMessage = getUserAgent();
    int contentLength = strlen(contentMessage);

    // The cache message
    char* cacheMessage = getCacheMessage(0);
    int cacheLength = strlen(cacheMessage);

    // Copy together all the strings
    char* finalString;
    finalString = malloc(deliveryLength + connectionLength + dateLength + cacheLength  + contentLength + 1);
    strcpy(finalString, deliveryCodeMessage);
    strcat(finalString, connectionMessage);
    strcat(finalString, dateMessage);
    strcat(finalString, cacheMessage);
    strcat(finalString, "\n");
    strcat(finalString, contentMessage);

    sendToClient(finalString,strlen(finalString));

    // Free memory
    free(finalString);

}
void handleCloseRequest(){
    printf("close");
}
void handleAddCartRequest(char* querystring, int querystringlength){
    printf("addcart");
}
void handleDelCartRequest(char* querystring){
    printf("delcart");
}
void handleCheckoutRequest(){
    printf("checkout");
}
void handleGetFileRequest(char* querystring, int querystringlength){
    printf("getfile");
}
void handlePutFileRequest(char* putfilestring,char* contentstring,int putfilelength,int contentstringlength){

}

void handleRedirectRequest(char* querystring, int querystringlength){
    printf("redirect");
}
