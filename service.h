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
char* getDeliveryCode(int statusCode);
char* getConnectionMessage(int command);
char* getCacheMessage(int command);
char* getDateMessage();
int getArraySize(char* array);
void handleServerTimeRequest();
void sendToClient(char* content, int contentLength);
void handleLoginRequest();
void handleLogoutRequest();
void handleBrowserRequest();
void handleCheckoutRequest();
void handleCloseRequest();
void handleAddCartRequest();
void handleDelCartRequest();
void handleRedirectRequest();
void handleGetFileRequest();
void throw404();
 void parseClose(char* buffer);
 void parseCheckout(char* buffer) ; 
  void parseBrowser(char* buffer);
 void parseRedirect(char* buffer);
 void parseGetFile(char* buffer);
void parseAddCart(char* buffer);
void parseDelCart(char* buffer);
 void parseCheckout(char* buffer);
 void parseClose(char* buffer);
#endif
