/*
 * File: service.h
 */

#ifndef _SERVICE_H_
#define _SERVICE_H_

void handle_client(int socket);
void receive(int socket, char* commandBuffer);
char * resizeCommandArray(char* commandBuffer);
void flushCommandBuffer(char* commandBuffer);
void parseCommand(char* buffer, int buffersize);
int requestIsValid(char* buffer, int buffersize);
void printBuffer(char* buffer, int buffersize);
char* getDeliveryCode(int statusCode);
char* getConnectionMessage(int command);
char* getCacheMessage(int command);
char* getDateMessage();
int getArraySize(char* array);
void handleServerTimeRequest();
void sendToClient(char* content, int contentLength);
void handleLoginRequest(char* querystring, int querystringsize);
void handleLogoutRequest();
void handleBrowserRequest();
void handleCheckoutRequest();
void handleCloseRequest();
void handleAddCartRequest();
void handleDelCartRequest();
void handleRedirectRequest();
void handleGetFileRequest();
void throw404();
void parseLogout(char* buffer, int buffersize, int startofcommand);
 void parseLogin(char* buffer, int buffersize, int startofcommand);
 void parseCheckout(char* buffer, int buffersize, int startofcommand);
  void parseBrowser(char* buffer, int buffersize, int startofcommand);
 void parseRedirect(char* buffer, int buffersize, int startofcommand);
 void parseGetFile(char* buffer, int buffersize, int startofcommand);
void parseAddCart(char* buffer, int buffersize, int startofcommand);
void parseDelCart(char* buffer, int buffersize, int startofcommand);
 void parseCheckout(char* buffer, int buffersize, int startofcommand);
 void parseClose(char* buffer, int buffersize, int startofcommand);
#endif
