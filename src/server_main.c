#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8080

static const char* role_name(char role_byte) {
    return (role_byte == 'R') ? "RED" : "BLACK";
}

int main() {

    int server_fd, client1, client2;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("setsockopt failed\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        printf("Bind failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 2) < 0) {
        printf("Listen failed\n");
        exit(EXIT_FAILURE);
    }

    printf("[server] listening on port %d\n", PORT);
    printf("[server] waiting for players...\n");

    client1 = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client1 < 0) {
        printf("Client1 connection failed\n");
        exit(EXIT_FAILURE);
    }
    printf("[server] player 1 connected (fd=%d)\n", client1);

    client2 = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (client2 < 0) {
        printf("Client2 connection failed\n");
        exit(EXIT_FAILURE);
    }
    printf("[server] player 2 connected (fd=%d)\n", client2);

    srand(time(NULL));
    int role = rand() % 2;
    // Role protocol: single byte => 'R' for red, 'B' for black.
    char role1 = (role == 0) ? 'R' : 'B';
    char role2 = (role == 0) ? 'B' : 'R';
    send(client1, &role1, 1, 0);
    send(client2, &role2, 1, 0);
    printf("[server] roles assigned: player1=%s, player2=%s\n", role_name(role1), role_name(role2));

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client1, &readfds);
        FD_SET(client2, &readfds);

        int maxfd = (client1 > client2 ? client1 : client2) + 1;
        int activity = select(maxfd, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            break;
        }

        if (FD_ISSET(client1, &readfds)) {
            int valread = read(client1, buffer, sizeof(buffer));
            if (valread <= 0) {
                printf("[server] player 1 disconnected\n");
                break;
            }
            printf("[server] relay p1->p2: %.*s", valread, buffer);
            send(client2, buffer, valread, 0);
        }

        if (FD_ISSET(client2, &readfds)) {
            int valread = read(client2, buffer, sizeof(buffer));
            if (valread <= 0) {
                printf("[server] player 2 disconnected\n");
                break;
            }
            printf("[server] relay p2->p1: %.*s", valread, buffer);
            send(client1, buffer, valread, 0);
        }
    }

    printf("[server] shutting down\n");

    close(client1);
    close(client2);
    close(server_fd);

    return 0;
}