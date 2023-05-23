#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function of checking arguments on right quantity.
void checkArguments(int quantity, char* text, char* ip) {
    if (quantity != 4) {
        fprintf(stderr, text, ip);
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    // client socket
    int cs;
    // echo server address
    struct sockaddr_in address;
    // echo server port
    unsigned short port;
    // server ip address
    char* ip;
    checkArguments(argc, "Usage: %s <Server IP> <Server Port> <Number of areas>\n", argv[0]);
    // receiving of arguments
    ip = argv[1];
    port = atoi(argv[2]);
    int numberOfAreas = atoi(argv[3]);
    // creating of the new socket using TCP
    cs = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // checking of socket
    if (cs < 0) {
        perror("Process of socket was failed");
        exit(1);
    }
    // create the server address structure
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);
    // establish the connection to the echo server
    if (connect(cs, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Process of connect was failed");
        exit(1);
    }
    int *areas = (int*) malloc(sizeof(int) * numberOfAreas);
    // endless loop
    while (1) {
        // receiving from server
        recv(cs, areas, sizeof(int) * numberOfAreas, 0);
        for (int i = 0; i < numberOfAreas; i++) {
            printf("%d\n", areas[i]);
        }
        sleep(1);
    }
    free(areas);
    // closing of the client socket
    close(cs);
    return 0;
}

