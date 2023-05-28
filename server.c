#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>

void *receiver(void *arg);
void *handleClient(void *server_fd);

const int MAX_CLIENT = 2;
const int BUFFER_SIZE = 1024;
int server;
int players[MAX_CLIENT];
int connected_player;
int incoming_score;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    pthread_mutex_init(&lock, NULL);

    int portNumber;
    socklen_t len;
    struct sockaddr_in servAdd;

    if (argc != 2) {
        fprintf(stderr, "Call model: %s <Port#>\n", argv[0]);
        exit(0);
    }
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "[-] Could not create socket\n");
        exit(1);
    } else {
        fprintf(stderr, "[+] Socket Created\n");
    }
    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    sscanf(argv[1], "%d", &portNumber);
    servAdd.sin_port = htons((uint16_t) portNumber);
    bind(server, (struct sockaddr *) &servAdd, sizeof(servAdd));

    if (listen(server, 6) < 0) {
        printf("[-] Error in binding.\n");
        return (EXIT_FAILURE);
    }

    pthread_mutex_t score_mutex;
    pthread_mutex_init(&score_mutex, NULL);

    pthread_t receiver;
    if(pthread_create(&receiver, NULL, &handleClient, &server) != 0) {
        fprintf(stderr, "\nFailed to create mining thread\n");
        return (EXIT_FAILURE);
    };

    if (pthread_join(receiver, NULL) != 0) {
        fprintf(stderr, "\nFailed to join socket thread\n");
        return 1;
    }
    
    pthread_mutex_destroy(&score_mutex);
    return 0;
}

void *handleClient(void *server_fd) {
    int client;
    int server = *((int *)server_fd);
    printf("\n[+] Server ready!\n");

    while (1) {
        client = accept(server, (struct sockaddr *) NULL, NULL);
        char waiting_char[1000];
        sprintf(waiting_char, "Waiting for players (%d/%d)", connected_player, MAX_CLIENT);

        if(connected_player > MAX_CLIENT) {
            write(client, "Server is full", 100);
            close(client);
            continue;
        }
        pthread_mutex_lock(&lock);
        connected_player++;
        pthread_mutex_unlock(&lock);
        write(client, waiting_char, sizeof(waiting_char));

        pthread_t thread[MAX_CLIENT];
        pthread_create(&thread[connected_player - 1], NULL, &receiver, (void *) &server);

        for (int i = 0; i < MAX_CLIENT; i++) {
            if (pthread_join(thread[i], NULL) != 0) {
                fprintf(stderr, "\nFailed to join socket thread #%d\n", i);
                return (void*)1;
            }
        }
    }

}

void *receiver(void *local) {
    int local_fd = *((int *)local);
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    int valread;
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    int k = 0;

    while (1) {
        k++;

        ready_sockets = current_sockets;

        int client_socket;

        if ((client_socket = accept(local_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        
        printf("[+] Client #%d connected.\n", k);
        pthread_mutex_lock(&lock);
        players[k] = client_socket;
        connected_player++;
        pthread_mutex_unlock(&lock);
    
        unsigned char data[BUFFER_SIZE];
        memcpy(data, buffer, BUFFER_SIZE);
        valread = recv(k, data, BUFFER_SIZE, 0);
        if(valread == 0) {
            printf("[-] Client #%d disconnected.\n", k);
            players[k] = 0;
            connected_player--;
            continue;
        }
        if(valread > 0) {
            printf("[+] Client #%d received -> %s\n", k, buffer);
        }
    }

    pthread_exit(NULL);
}