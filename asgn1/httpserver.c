#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <err.h>
#include "bind.h"
#include <sys/socket.h>
#include <sys/stat.h>
/*
struct requestStruct{
    char command[8];
    char filename[1024];
    char version[16];
    char header[1024];
    char message[1024];
    int contentLength; 
};
struct responseStruct{
    char filename[1024];
    int status;
    char command[8];
    char message[20];
    int contentLength; 
};
*/
typedef struct messageStruct{
    int getNum;
    char messageGet[2048];
}messageStruct;
void badRequest(int socket, char* version){
    char *badStatusLine = (char*)malloc(sizeof(char)*1024);
    char *badHeaderField =  (char*)malloc(sizeof(char)*2048);
    char *badMessageBody =  (char*)malloc(sizeof(char)*1024);
    sprintf(badStatusLine, "%s %d %s", version, 400, "Bad Request\r\n");
    sprintf(badHeaderField, "%s%d\r\n\r\n", "Content-Length: ", 12);
    sprintf(badMessageBody, "%s", "Bad Request\r\n");
    write(socket, badStatusLine, strlen(badStatusLine));
    write(socket, badHeaderField, strlen(badHeaderField));
    write(socket, badMessageBody, strlen(badMessageBody));
    free(badStatusLine);
    free(badHeaderField);
    free(badMessageBody);
}
// only put REQUESTS includes a message-body
void put(int socket, char* filename,char* version, char* message, int length){
   int exist = 0;
   //printf("file name is %s\n", filename);
   int status;
   char* tmp = (char*)malloc(sizeof(char)*1024);
   strcpy(tmp, filename);
   memmove(tmp, tmp+1, strlen(tmp));
   if(access(tmp, F_OK) == 0){                     //file exists
       exist = 1;
   } 
   struct stat statbuf;
   stat(tmp, &statbuf);
   int ret = 0;
   if(S_ISDIR(statbuf.st_mode) == 1){
       status = 403;
   } else{
       int fd = open(tmp, O_WRONLY|O_CREAT|O_TRUNC);
       //printf("put fd is %d \n", fd);
       int messSize = strlen(message);
       if(length> messSize){
        length = messSize;
       }
       ret =write(fd, message, length);
       close(fd);
       printf("Return of put write is %d\n", ret);
    }

   if((ret >= 0) &&(status != 403)){
      if(exist){
        status = 200;
      } else{
        status = 201;
      } 
   } else{
        status = 400;
   }
   char *statusLine = (char*)malloc(sizeof(char)*1024);
   char *headerField =  (char*)malloc(sizeof(char)*2048);
   char *messageBody =  (char*)malloc(sizeof(char)*1024);
   switch(status){
      case 201:
            sprintf(statusLine, "%s %d %s", version, status, "Created\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", 8);
            sprintf(messageBody, "%s", "Created\n");
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));
            break;
        case 200:
            sprintf(statusLine, "%s %d %s", version, status, "OK\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", 3);
            sprintf(messageBody, "%s", "OK\n");
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));
            break;
        case 403:
            sprintf(statusLine, "%s %d %s", version, status, "Forbidden\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", 10);
            sprintf(messageBody, "%s", "Forbidden\n");
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));
            break;
        default:
            
            printf("Status is %d and Bad Request for Put\n", status);
            sprintf(statusLine, "%s %d %s", version, status, "Bad Request\r\n");
            sprintf(headerField, "%s %d\r\n\r\n", "Content-Length: ", 12);
            sprintf(messageBody, "%s", "Bad Request\n");
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));
   }
      free(tmp);
    free(statusLine);
    free(headerField);
    free(messageBody);
}
// get icludes a message-body for RESPONSE
void get(messageStruct *curlMessage, int socket, char* filename,char* version){
   int status;
   char* tmp = (char*)malloc(sizeof(char)*1024);
   char *message =  (char*)malloc(sizeof(char)*4086);
   strcpy(tmp, filename);
   memmove(tmp, tmp+1, strlen(tmp));  
   if(access(tmp, F_OK) == 0){     //file exists   
       struct stat statbuf;
       stat(tmp, &statbuf);
       if(S_ISDIR(statbuf.st_mode) == 1){
          status = 403;
       } else{
           int fd = open(tmp, O_RDONLY);
           int readLength = read(fd, message, 4086);
           if(readLength < 0){
              status = 400;
           }
           strcpy(curlMessage->messageGet, message);
           curlMessage->getNum = 1;
           printf("read length is %d", readLength);
           status = 200;
           close(fd);
       }
   } else{
       status = 404;
   }
   int messageLen = strlen(message);
    char *statusLine = (char*)malloc(sizeof(char)*1024);
    char *headerField =  (char*)malloc(sizeof(char)*2048);
    char *messageBody =  (char*)malloc(sizeof(char)*4086);
    switch(status){
       case 403:
            sprintf(statusLine, "%s %d %s", version, status, "Forbidden\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", 10);
            sprintf(messageBody, "%s", "Forbidden\n");
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));
            break;
       case 404:
            sprintf(statusLine, "%s %d %s", version, status, "Not Found\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", 10);
            sprintf(messageBody, "%s", "Not Found\n");
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));
            break;
        case 200:
            sprintf(statusLine, "%s %d %s", version, status, "OK\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", messageLen);
            sprintf(messageBody, "%s", message);
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));
            break;
        default:
            sprintf(statusLine, "%s %d %s", version, status, "Bad Request\r\n");
            sprintf(headerField, "%s %d\r\n\r\n", "Content-Length: ", 12);
            sprintf(messageBody, "%s", "Bad Request\n");
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            write(socket, messageBody, strlen(messageBody));

   }
   free(message);
   free(tmp);
    free(statusLine);
    free(headerField);
    free(messageBody);
}
void head(int socket, char* filename,char* version){
   int status;
    char* tmp = (char*)malloc(sizeof(char)*1024);
   char *message =  (char*)malloc(sizeof(char)*8192);
   strcpy(tmp, filename);
   memmove(tmp, tmp+1, strlen(tmp));
   if(access(tmp, F_OK) == 0){                     //file exists
       struct stat statbuf;
       stat(tmp, &statbuf);
       if(S_ISDIR(statbuf.st_mode) == 1){
          status = 403;
       } else{
           int fd = open(tmp, O_RDONLY);
           int readLength = read(fd, message, 8192);
           if(readLength < 0){
              status = 400;
           }
           status = 200;
           close(fd);
       }
   } else{
       status = 400;
   }
   int messageLen = strlen(message);
    char *statusLine = (char*)malloc(sizeof(char)*1024);
    char *headerField =  (char*)malloc(sizeof(char)*2048);
    switch(status){
       case 403:
            sprintf(statusLine, "%s %d %s", version, status, "Forbidden\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", 10);
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            break;
        case 404:
            sprintf(statusLine, "%s %d %s", version, status, "Not Found\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", 10);
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            break;
        case 200:
            sprintf(statusLine, "%s %d %s", version, status, "OK\r\n");
            sprintf(headerField, "%s%d\r\n\r\n", "Content-Length: ", messageLen);
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
            break;
        default:
            sprintf(statusLine, "%s %d %s", version, status, "Bad Request\r\n");
            sprintf(headerField, "%s %d\r\n\r\n", "Content-Length: ", 12);        
            write(socket, statusLine, strlen(statusLine));
            write(socket, headerField, strlen(headerField));
   }
   free(tmp);
   free(message);
   free(statusLine);
   free(headerField);
}
int main(int argc, char* argv[]){
    int err = 0;
    int server;
    if(argc < 2){
        warnx("wrong arguments: %s port_num\nusage: %s <port>", argv[0],argv[0]);
        err = 1;
    } 
    else {
        char *tmp;
        int port = strtol(argv[argc-1], &tmp,10);
        server = create_listen_socket(port);
        int fd;
        messageStruct curlMessage;
        for(;;){
            //printf("The port number is %d\n", port);
            //*printf("The accept return value is %d\n", fd);
            fd = accept(server, NULL, NULL);
            int lineSize = 4086;
            char *line = (char *)malloc(lineSize);
            read(fd, line, lineSize);
            char *command = (char*)malloc(sizeof(char)*16);    //8 char or fewer
            sscanf(line, "%s ", command);
            //printf("%s ", command);
            if(strcmp(command, "PUT") == 0){
            printf("%s\n\n\n", line);
                //PUT /1234.txt HTTP/1.1\r\nContent-Length: 12\r\n\r\nHello World!
                char *filename = (char*)malloc(sizeof(char)*1024);
                char *version = (char*)malloc(sizeof(char)*16);
                char *rest = (char*)malloc(sizeof(char)*2048);
                int contentL;
                int read = sscanf(line, "%s %s %s%*[\r\n]%2048c", command, filename, version, rest);
            //printf("rest is %s\n", rest);
                char *trash = (char*)malloc(sizeof(char)*2048);
                int restlen= strlen(rest);
                char *contentString = strstr(rest, "Content-Length: ");
                sscanf(contentString, "%s %d", trash, &contentL);
            //printf("messlen is %d", restlen);
                int last = 0;
                for(int i = 0; i < (restlen-4); i++){
                    if((rest[i]=='\r')&&
                    (rest[i+1]=='\n')&&
                    (rest[i+2]=='\r')&&
                    (rest[i+3]=='\n') ){
                        last = i + 4;
                    }
                }
                memmove(rest, rest+last, strlen(rest)); 
              printf("\nlast is %d \n", last);
              printf("message is %s\n", rest);
            //printf("%s %s %s %d %s\n\n", command, filename, version, contentL,rest);
                if(read < 3){
                ////printf("READ WRONG IT IS %d \n", read);
                    badRequest(fd, "HTTP/1.1");
                } else if(strcmp(version, "HTTP/1.1") != 0 ){
                    badRequest(fd, "HTTP/1.1");
                } else if(last == 0){
                    put(fd, filename, version, curlMessage.messageGet, contentL);
                }
                else if(contentString == NULL){
                    badRequest(fd, "HTTP/1.1");
                }
                else{
                    put(fd, filename,version,  rest, contentL);
                //printf("put executed");
                }
                free(trash);
                free(rest);
                free(version);
                free(filename);

            } else if (strcmp(command, "GET") == 0){
                //GET /1234.txt HTTP/1.1\r\n\r\n
                char *filename = (char*)malloc(sizeof(char)*1024);
                char *version = (char*)malloc(sizeof(char)*16);
                char *garbage =  (char*)malloc(sizeof(char)*1024);
                memset(garbage, 0, strlen(garbage));
                int read = sscanf(line, "%s %s %s%*[\r\n\r\n]%s", command, filename, version,garbage);
                //printf("read is %d", read);
                //printf("%s %s %s %s", command, filename, version, garbage);
                int garbageSize = strlen(garbage);
                if(read < 3){
                    badRequest(fd, "HTTP/1.1");
                } else if(strcmp(version, "HTTP/1.1") != 0 ){
                    badRequest(fd, "HTTP/1.1");
                } else if( ( (strcmp(garbage, "User-Agent:") != 0) && (strcmp(garbage, "Accept:") != 0)&& (strcmp(garbage, "Host:") != 0))&& (garbageSize !=0 )){
                    //printf("garbageSize is %d\n", garbageSize);
                    //printf("garbage is %s\n", garbage);
                    badRequest(fd, "HTTP/1.1");
                }
                else{
                    get(&curlMessage, fd, filename, version);
                    printf("get was %s", curlMessage.messageGet);
                }
                free(garbage);
                free(filename);
                free(version);
            } else if (strcmp(command, "HEAD") == 0){
                //HEAD /1234.txt HTTP/1.1\r\n\r\n
                char *filename = (char*)malloc(sizeof(char)*1024);
                char *version = (char*)malloc(sizeof(char)*16);
                char *garbage =  (char*)malloc(sizeof(char)*1024);
                memset(garbage, 0, strlen(garbage));
                int read = sscanf(line, "%s %s %s%*[\r\n\r\n]%s", command, filename, version, garbage);
                int garbageSize = strlen(garbage);
                //printf("The version is %s\n", version);
                //printf("the length of garbage is %d", garbageSize);
                //printf("%s", garbage);
                if(read < 3){
                    badRequest(fd, version);
                } else if(strcmp(version, "HTTP/1.1") != 0 ){
                    badRequest(fd, "HTTP/1.1");
                } else if( ( (strcmp(garbage, "User-Agent:") != 0) && (strcmp(garbage, "Accept:") != 0)&& (strcmp(garbage, "Host:") != 0))&& (garbageSize !=0 )){
                    //printf("garbageSize is %d\n", garbageSize);
                    badRequest(fd, "HTTP/1.1");
                } 
                else {
                    head(fd, filename, version);
                }
                free(filename);
                free(version);
                free(garbage);
            } else{
               printf("Bad Request before Methods\n");
                badRequest(fd,"HTTP/1.1");
            }
            free(command);
            free(line);
            close(fd);
        }
        close(server);
        //int true = 1;
       // setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));
    }
      
    return err;
}
