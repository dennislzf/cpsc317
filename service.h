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
void handleServerTimeRequest(int statusCode);
char* getDeliveryCode(int statusCode);
int getArraySize(char* array);
void sendToClient(char* content, int contentLength);
void handleServerTime(int i);
void handleLogin(int i);

#endif
