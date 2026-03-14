#include<stdlib.h>
#include<stdio.h>
#include <stdbool.h>
#include<sys/socket.h>    
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<signal.h>
#include<errno.h>

#define PORT 5000
#define MAXLINE 1024

// int max(int x, int y){
//     if(x>y) return x;
//     else return y;
// }

int main(){
    char buffer[MAXLINE];
    struct sockaddr_in address; //client ka address
    socklen_t addrlen= sizeof(address);
    int sockfd= socket(AF_INET, SOCK_STREAM, 0); //return file descriptor if successful
    if(sockfd<0){
        perror("socket failed!");
        exit(EXIT_FAILURE);;
    }

    memset(&address, 0, sizeof(address)); //we generally initialize the structures or variables before using them
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr*)&address , sizeof(address))<0){
        perror("Bind failed!");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd , 1)<0){
        perror("Listen failed!");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client... \n");

    int client_socket= accept(sockfd, (struct sockaddr*)&address , &addrlen);
    //accept expects third parameter as pointer
    
    if (client_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected!\n");

    memset(buffer, 0 , sizeof(buffer));
    read(client_socket, buffer, sizeof(buffer));
    printf("Message from client: ");
    puts(buffer);
    char *msg = "Hello client!";
    write(client_socket, msg, strlen(msg));
    close(client_socket);
    close(sockfd);

    return 0;
}