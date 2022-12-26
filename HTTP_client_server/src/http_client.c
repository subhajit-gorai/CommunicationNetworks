//
// Created by subhajit on 25/09/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 10000 // max number of bytes we can get at once


void parseUrl(char *url, char *host, char *port, char *location) {
    int i, len, state,h = 0, p = 0, l = 1, start = 7;
    state = 1;
    location[0] = '/';
    len = strlen(url);
    if(8 < len && url[7] == '/') start = 8;
    for(i = start ; i < len ; i++) {
        if(state == 1)  {
            if(url[i] == ':') state = 2;
            else if(url[i] == '/') state = 3;
            else {
                host[h++] = url[i];
            }
        }else if(state == 2) {
//            PORT
            if(url[i] == '/') state = 3;
            else port[p++] = url[i];
        }else if(state == 3) {
//            location
            location[l++] = url[i];
        }

    }
    if(p == 0) {
        port[0] = '8';
        port[1] = '0';
        port[2] = '\0';
    }else
        port[p] = '\0';
    host[h] = '\0';
    location[l] = '\0';
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    int count = 0;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    FILE *outputFile;
    char *filename = "output";
    bool breakFlag = false;
    if (argc != 2) {
        fprintf(stderr,"the location of the file is not specified\n");
        exit(1);
    }
    char *url = argv[1];
    char *host, *port, *location;
    host = (char *) malloc(17);
    port = (char *) malloc(7);
    location = (char *) malloc(100);
    parseUrl(url, host, port, location);
    memset(&hints, 0, sizeof hints);
    printf("host is #%s#port is #%s# location is #%s# \n", host, port, location);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
//    send(new_fd, "Hello, world!", 13, 0) == -1)
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    char *clientData;
//            "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: fetch.c\r\n\r\n";
//    char *cleintData = "GET / HTTP/1.1\r\nUser-Agent: Wget/1.12 (linux-gnu)\r\nHost: %s:%s\r\nConnection: Keep-Alive\r\n";
    asprintf(&clientData, "GET / HTTP/1.0\r\nUser-Agent: http_client.c\r\nHost: %s:%s\r\n\r\n", host, port);
//    asprintf(&cleintData, ");
    printf("clientData \n%s#\n#\n", clientData);
    if(send(sockfd, clientData, strlen(clientData), 0) == -1) {
        perror("send");
    }
//    shutdown(sockfd, SHUT_WR);
    outputFile = fopen(filename,"w");

    while (1) {
        printf("inside loop !! \n");
        numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
        printf("after receive\n");
        if(numbytes == - 1) {
            perror("rec");
            exit(1);
        }
        printf("numbytes is %d\n", numbytes);
        if(numbytes == 0)// two new line values marks the end of the input
        {

            count++;
            printf("count %d\n", count);
            if(count == 10) {
                buf[numbytes] = '\0';
                numbytes++;
                breakFlag = true;

            }
        }
//        buf[numbytes] = '\0';

        if(numbytes > 0 ) {
//            buf[numbytes] = '\0';
            fprintf(outputFile, buf);
        }
//        printf("client: received '%s'\n",buf);
        if(breakFlag) break;

    }
    close(sockfd);

    return 0;
}

