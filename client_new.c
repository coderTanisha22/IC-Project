#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>    
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<signal.h>
#include<errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>  

#define PORT 5000
#define MAXLINE 1024

int main(){
    char* message= "Hello Server";
    char buffer[MAXLINE];

    struct sockaddr_in address; //server ka address
    int addrlen= sizeof(address);
    int sockfd= socket(AF_INET, SOCK_STREAM, 0); //return file descriptor if successful
    if(sockfd<0){
        perror("socket failed!");
        exit(EXIT_FAILURE);;
    }
    memset(&address, 0, sizeof(address)); 
    //memset is just a memory filling function

    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    if(connect(sockfd,(struct sockaddr*)&address, sizeof(address))<0){
        perror("\nConnection failed!\n");
        exit(EXIT_FAILURE);
    }

    memset(buffer,0, sizeof(buffer)); // just clearing the buffer
    strcpy(buffer, message);
    write(sockfd, buffer, strlen(message));
    printf("Message from server: ");
    read(sockfd, buffer, sizeof(buffer));
    puts(buffer); //print whatever read
    close(sockfd);
}