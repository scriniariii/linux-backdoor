#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 1234
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[!] Error while creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server addres
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Binding socket to port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[!] Error while binding socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[+] Socket bonded to port %d\n", PORT);

    // Listening connections
    if (listen(server_fd, 5) < 0) {
        perror("[!] Error while listeng");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[+] Waiting for connections...\n");

    // Accept client connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("[!] Error accepting connection");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[+] Connection established with IP: %s, Port: %d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Interact with the client
    while (1) {
        printf("> ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0'; // Delete line break
        if (strcmp(command, "quit") == 0) {
            printf("[+] Closing connection...\n");
            close(client_fd);
            close(server_fd);
            exit(0);
        }

        send(client_fd, command, strlen(command), 0);

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("%s\n", buffer);
        }
    }

    return 0;
}
