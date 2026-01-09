#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
/// made by GorGor
// very helpful utility for connectivity testing and more 
#pragma comment(lib, "ws2_32.lib")

int main (int argc, char *argv[]) {
    WSADATA _DATA;
    if (WSAStartup(MAKEWORD(2,2), &_DATA) != 0) return 1;

    if(argc < 1) {
        printf("Use -h for help menu\n");
        return 1;
    }

    if(strcmp(argv[1], "-h") == 0){
        printf("A) UDP listener: -UDP -host=127.0.0.1 -port=4444\n");
        printf("B) TCP listener: -TCP -host=127.0.0.1 -port=4444\n");
        printf("C) Sending: -send=\"hello\" -host=127.0.0.1 -port=5555 -UDP or -TCP\n");
    }
    else if(argc >= 4 && (strcmp(argv[1], "-UDP") == 0 || strcmp(argv[1], "-TCP") == 0)){
        int is_udp = (strcmp(argv[1], "-UDP") == 0);
        SOCKET s = socket(AF_INET, is_udp ? SOCK_DGRAM : SOCK_STREAM, 0);
        struct sockaddr_in Addr, client;
        int clientlen = sizeof(client);
        char ip[16]={0}, buffer[1024];
        int port=0;

        for (int i = 2; i < argc; i++) {
            if (strncmp(argv[i], "-host=", 6) == 0) strncpy(ip, argv[i] + 6, 15);
            else if (strncmp(argv[i], "-port=", 6) == 0) port = atoi(argv[i] + 6);
        }

        Addr.sin_addr.s_addr = inet_addr(ip);
        Addr.sin_family = AF_INET;
        Addr.sin_port = htons(port);

        if(bind(s, (struct sockaddr*)&Addr, sizeof(Addr)) == SOCKET_ERROR) return 1;

        if(!is_udp) listen(s, SOMAXCONN);
        printf("Listening for %s on %s:%d\n", is_udp ? "UDP" : "TCP", ip, port);

        while(1){
            memset(buffer, 0, sizeof(buffer));
            if(is_udp) {
                int x = recvfrom(s, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&client, &clientlen);
                if(x > 0) printf("[%s] Recv: %s\n", inet_ntoa(client.sin_addr), buffer);
            } else {
                SOCKET client_s = accept(s, (struct sockaddr*)&client, &clientlen);
                int x = recv(client_s, buffer, sizeof(buffer)-1, 0);
                if(x > 0) printf("[%s] Recv: %s\n", inet_ntoa(client.sin_addr), buffer);
                closesocket(client_s);
            }
        }
    }
    else if(argc >= 4 && strncmp(argv[1], "-send", 5) == 0){
        struct sockaddr_in Addr;
        char ip[16]={0}, buffer[1024]={0};
        int port=0, use_udp = 1;

        for (int i = 1; i < argc; i++) {
            if (strncmp(argv[i], "-host=", 6) == 0) strncpy(ip, argv[i] + 6, 15);
            else if (strncmp(argv[i], "-port=", 6) == 0) port = atoi(argv[i] + 6);
            else if (strncmp(argv[i], "-send=", 6) == 0) strncpy(buffer, argv[i] + 6, sizeof(buffer)-1);
            else if (strcmp(argv[i], "-TCP") == 0) use_udp = 0;
        }

        Addr.sin_addr.s_addr = inet_addr(ip);
        Addr.sin_family = AF_INET;
        Addr.sin_port = htons(port);

        SOCKET s = socket(AF_INET, use_udp ? SOCK_DGRAM : SOCK_STREAM, 0);
        if(use_udp) {
            sendto(s, buffer, strlen(buffer), 0, (struct sockaddr*)&Addr, sizeof(Addr));
            printf("[UDP] Sent to %s:%d\n", ip, port);
        } else {
            if(connect(s, (struct sockaddr*)&Addr, sizeof(Addr)) != SOCKET_ERROR) {
                send(s, buffer, strlen(buffer), 0);
                printf("[TCP] Sent to %s:%d\n", ip, port);
            } else printf("Connect Error: %d\n", WSAGetLastError());
        }
        closesocket(s);
    }

    WSACleanup();
    return 0;
}