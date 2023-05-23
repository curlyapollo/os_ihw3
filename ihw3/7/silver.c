#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// Maximum outstanding connection requests
#define MAXPENDING 7

// mutex
pthread_mutex_t mtx;
// all sectors where pirates are searching
int* areas;
// flag if treasure found
int wasFound = 0;

// structure with pirates' arguments
typedef struct piratesArgs {
    int sock;
    int areas;
} piratesArgs;

// function of checking arguments on right quantity.
void checkArguments(int quantity, char* text, char* ip) {
    if (quantity != 5) {
        fprintf(stderr, text, ip);
        exit(1);
    }
}

// process which simulate process of searching the treasure
void *piratesProcess(void* args) {
    // pirate socket
    int sock;
    pthread_detach(pthread_self());
    // receiving of arguments
    sock = ((piratesArgs*)args)->sock;
    int numOfAreas = ((piratesArgs*)args)->areas;
    // zeroing of args
    free(args);
    int given[2];
    int taken[2];
    int found = 0;
    int currentArea = -1;
    // endless loop
    while (1) {
        // lock the mutex
        pthread_mutex_lock(&mtx);
        //traversal of the areas
        for (int i = 0; i < numOfAreas; i++) {
            if (areas[i] != 1) {
                currentArea = i;
                if (areas[i] == 2) {
                    found = 1;
                } else {
                    found = 0;
                }
                areas[i] = 1;
                break;
            }
        }
        // unlock the mutex
        pthread_mutex_unlock(&mtx);
        given[0] = currentArea;
        given[1] = found;
        // sending to server the answer
        send(sock, given, sizeof(given), 0);
        if (currentArea == -1) {
            break;
        }
        // receiving from server
        recv(sock, taken, sizeof(taken), 0);
        // lock the mutex again
        pthread_mutex_lock(&mtx);
        // condition when we found the treasure
        if (taken[0] == 1) {
            if (!wasFound) {
                printf("Treasure was found!\n");
                wasFound = 1;
            }
            for (int i = 0; i < numOfAreas; i++) {
                areas[i] = 1;
            }
        }
        // unlock the mutex
        pthread_mutex_unlock(&mtx);
        currentArea = -1;
    }
    // closing of the socket
    close(sock);
}

void *processOfListener(void* args) {
    // listener socket
    int sock;
    // client length
    int cl;
    // client socket
    int cs;
    // echo server address
    struct sockaddr_in c_address;
    // receiving of arguments
    pthread_detach(pthread_self());
    sock = ((piratesArgs *)args)->sock;
    int numberOfAreas = ((piratesArgs *)args)->areas;
    // zeroing of args
    free(args);
    listen(sock, MAXPENDING);
    // endless loop which ended with 'break'
    while (1) {
        cl = sizeof(c_address);
        cs = accept(sock, (struct sockaddr *) &c_address, &cl);
        while (1) {
            if (wasFound) {
                break;
            }
            // sending to server the answer
            send(cs, areas, numberOfAreas * sizeof(int), 0);
            sleep(1);
        }
        // closing of the client socket
        close(cs);
    }
}


int main(int argc, char *argv[]) {
    // echo server port
    unsigned short port;
    // echo listener port
    unsigned short l_port;
    // server socket
    int ss;
    // client socket
    int cs;
    // listener socket
    int ls;
    // client length
    int cl;
    // echo client address
    struct sockaddr_in c_address;
    // echo server address
    struct sockaddr_in s_address;
    // echo listener address
    struct sockaddr_in l_address;
    pthread_t IDofProcess;
    // initializing of the mutex
    pthread_mutex_init(&mtx, NULL);
    checkArguments(argc, "Usage:  %s <Client port> <Listener port> <Number of areas> <Area with treasure>\n", argv[0]);
    // receiving the arguments
    port = atoi(argv[1]);
    l_port = atoi(argv[2]);
    int numOfAreas = atoi(argv[3]);
    int areaWithTreasure = atoi(argv[4]) - 1;
    // allocation of memory
    areas = (int*) malloc(sizeof(int) * numOfAreas);
    for (int i = 0; i < numOfAreas; i++) {
        areas[i] = 0;
    }
    areas[areaWithTreasure] = 2;
    // creating of the new server socket using TCP
    ss = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // checking of socket
    if (ss < 0) {
        perror("Process of socket was failed.");
        exit(1);
    }
    // create the server address structure
    memset(&s_address, 0, sizeof(s_address));
    s_address.sin_family = AF_INET;
    s_address.sin_addr.s_addr = htonl(INADDR_ANY);
    s_address.sin_port = htons(port);
    // establish the binding to the echo server
    if (bind(ss, (struct sockaddr *) &s_address, sizeof(s_address)) < 0) {
        perror("Process of bind was failed.");
        exit(1);
    }
    // creating of the new listener socket using TCP
    ls = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // checking of socket
    if (ls < 0) {
        perror("Process of socket was failed.");
        exit(1);
    }
    // create the listener address structure
    memset(&l_address, 0, sizeof(l_address));
    l_address.sin_family = AF_INET;
    l_address.sin_addr.s_addr = htonl(INADDR_ANY);
    l_address.sin_port = htons(l_port);
    // establish the binding to the echo server
    if (bind(ls, (struct sockaddr *) &l_address, sizeof(l_address)) < 0) {
        perror("Process of bind was failed.");
        exit(1);
    }
    piratesArgs *l_args = (piratesArgs *) malloc(sizeof(piratesArgs));
    l_args->sock = ls;
    l_args->areas = numOfAreas;
    // attempt of creation of pthread
    if (pthread_create(&IDofProcess, NULL, piratesProcess, (void *) l_args) != 0) {
        perror("Creation of pthread was failed.");
        exit(1);
    }
    // listen for incoming connections
    listen(ss, MAXPENDING);
    while (1) {
        // receiving of arguments
        cl = sizeof(c_address);
        cs = accept(ss, (struct sockaddr *) &c_address, &cl);
        piratesArgs *args = (piratesArgs *) malloc(sizeof(piratesArgs));
        args->sock = cs;
        args->areas = numOfAreas;
        // attempt of creation of pthread
        if (pthread_create(&IDofProcess, NULL, piratesProcess, (void *) args) != 0) {
            perror("Creation of pthread was failed.");
            exit(1);
        }
    }
    free(areas);
    pthread_mutex_destroy(&mtx);
}
