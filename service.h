/*
 * File: service.h
 */

#ifndef _SERVICE_H_
#define _SERVICE_H_

void handle_client(int socket);
void receive(int socket, char* commandBuffer);
char * resizeCommandArray(char* commandBuffer);
void flushCommandBuffer(char* commandBuffer);
char* parseCommand(char* buffer, int buffersize);
int requestIsValid(char* buffer, int buffersize);
void printBuffer(char* buffer, int buffersize);
void handleServerTimeRequest();
char* getDeliveryCode(int statusCode);
char* getConnectionMessage(int command);
char* getDateMessage();
int getArraySize(char* array);
void sendToClient(char* content, int contentLength);
void handleLoginRequest();

#endif
