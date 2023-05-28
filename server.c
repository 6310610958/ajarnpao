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
int incoming_score[MAX_CLIENT];
int sent_score;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int id;
    int socket;
} Client;

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
    if (bind(server, (struct sockaddr *) &servAdd, sizeof(servAdd)) < 0)
    {
        printf("[-] Bind failed");
        return (EXIT_FAILURE);
    }

    if (listen(server, 6) < 0) {
        printf("[-] Listen failed.\n");
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
    pthread_t thread[MAX_CLIENT];
    int server = *((int *)server_fd);
    printf("[+] Server ready!\n");

    while (1) {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        client = accept(server, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        int id = connected_player;

        printf("[+] Client #%d connected.\n", id + 1);

        char waiting_char[BUFFER_SIZE];
        sprintf(waiting_char, "Waiting for players (%d/%d)", connected_player + 1, MAX_CLIENT);
        
        pthread_mutex_lock(&lock);
        players[id] = client;
        connected_player++;
        pthread_mutex_unlock(&lock);
        write(client, waiting_char, sizeof(waiting_char));

        if(connected_player == MAX_CLIENT) {
            printf("[+] Sending READY to all clients\n");
            for (int i = 0; i < MAX_CLIENT; i++) write(players[i], "READY", 6);
        }

        
        Client client_info;
        client_info.id = id;
        client_info.socket = client;

        pthread_create(&thread[connected_player - 1], NULL, &receiver, (void *) &client_info);

        if(connected_player == MAX_CLIENT) break;
    }

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (pthread_join(thread[i], NULL) != 0) {
            fprintf(stderr, "\nFailed to join socket thread #%d\n", i);
            return (void*)1;
        }
    }

    pthread_exit(NULL);
    return 0;
}

void *receiver(void *info) {
    Client client_info = *((Client *)info);
    
    char buffer[BUFFER_SIZE];
    while (1) {
        bzero(buffer, BUFFER_SIZE);

        int n = read(client_info.socket, buffer, BUFFER_SIZE);

        if (n < 0){
            perror("[-] ERROR");
            exit(1);
        }
        if(n == 0) break;

        printf("[+] Client #%d said (%d): %s \n", client_info.id + 1, n, buffer);

        int number = atoi(buffer);
        int random_number = rand() + number % 6;
        pthread_mutex_lock(&lock);
        sent_score++;
        incoming_score[client_info.id] = random_number;
        pthread_mutex_unlock(&lock);
        printf("[+] Client #%d score: %d\n", client_info.id + 1, random_number);

        if(sent_score >= MAX_CLIENT) {
            int max = 0;
            int id = 0;

            printf("[+] Sending result to all clients\n");
            // find the max score
            for (int i = 0; i < MAX_CLIENT; i++) {
                if(incoming_score[client_info.id] > max) {
                    max = incoming_score[client_info.id];
                    id = i;
                }
            }
            for (int i = 0; i < MAX_CLIENT; i++) {
                if(i == id) {
                    write(players[i], "WIN", 3);
                    continue;
                };
                write(players[i], "LOSE", 4);
            }
        };

        // escape this loop, if the client sends message "quit"
        if (!bcmp(buffer, "quit", 4)) break;
    }

    pthread_exit(NULL);
}