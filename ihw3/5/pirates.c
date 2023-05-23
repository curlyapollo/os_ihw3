#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function of checking arguments on right quantity.
void checkArguments(int quantity, char* text, char* ip) {
    if (quantity != 3) {
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
    checkArguments(argc, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
    // receiving of arguments
    ip = argv[1];
    port = atoi(argv[2]);
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
    int taken[2];
    int given[2];
    // endless loop
    while (1) {
        // receiving from server
        recv(cs, taken, sizeof(taken), 0);
        // search over
        if (taken[0] == -1)  {
            break;
        }
        printf("Scanning of sector №%d\n", taken[0]);
        sleep(1 + rand() % 3);
        if (taken[1]) {
            printf("Treasure was found!\n");
            given[0] = 1;
        } else {
            given[0] = 0;
        }
        // sending to server
        send(cs, given, sizeof(given), 0);
    }
    printf("Search is over.\n");
    // closing of the client socket
    close(cs);
    return 0;
}
