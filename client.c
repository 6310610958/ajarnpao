#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *handleServer(void *arg);

// Ref: https://stackoverflow.com/a/26589552
int ReadUntilEOL(void) {
    char ch;
    int count;
    while ((count = scanf("%c", &ch)) == 1 && ch != '\n')
    ; // Consume char until \n or EOF or IO error
    return count;
}

int BUFFER_SIZE = 1024;
int ready_to_play = 0;
int main(int argc, const char *argv[])
{
    char message[100];
    int server;
    socklen_t len;
    struct sockaddr_in servAdd;
    int dice;
    int32_t point;
    int size = sizeof(point);

    if (argc != 3)
    {
        printf("Call model:%s <IP> <Port#>\n", argv[0]);
        exit(EXIT_FAILURE);
        return 0;
    }
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("[-] Cannot create socket\n");
        exit(EXIT_FAILURE);
        return 0;
    }
    int port = 0;
    if(argv[2] != NULL) sscanf(argv[2], "%d", &port);

    servAdd.sin_family = AF_INET;
    // store this IP address in sa:
    inet_pton(AF_INET, argv[1], &(servAdd.sin_addr));
    servAdd.sin_port = htons(port);

    printf("[+] Socket Created.\n");

    if (connect(server, (struct sockaddr *)&servAdd, sizeof(servAdd)) < 0)
    {
        printf("[-] Connection Failed.\n");
        return (EXIT_FAILURE);
    }
    printf("[+] Connected to server.\n");

    srand(time(NULL));

    pthread_t receiver;
    if(pthread_create(&receiver, NULL, &handleServer, &server)) {
        fprintf(stderr, "\nFailed to create client thread\n");
        return (EXIT_FAILURE);
    }

    if (pthread_join(receiver, NULL) != 0) {
        fprintf(stderr, "\nFailed to join socket thread\n");
        return 1;
    }
    exit(0);
}

int read_int() {
    int n;
    for (;;) {
        printf("Enter an integer: ");
        char NextChar = '\n';
        int count = scanf("%d%c", &n, &NextChar);
        if (count >= 1 && NextChar == '\n') 
            break;
        if (ReadUntilEOL() == EOF) 
            return 1;  // No valid input ever found
    }
    return n;
}

void *handleServer(void* args) {
    int server = *(int*)args;
    int n = 0;
    char buffer[BUFFER_SIZE];

    while(1) {
        bzero(buffer, BUFFER_SIZE);
        n = read(server, buffer, BUFFER_SIZE);

        if (n < 0){
            perror("[-] ERROR");
            exit(1);
        }

        if(n == 0) break;
        if(strcmp(buffer, "READY") == 0) {
            int number = read_int();
            
            char str[BUFFER_SIZE];
            sprintf(str, "%d", number);

            write(server, &str, sizeof(str));
            printf("[+] Sent nonce: %d\n", number);
        } else if (strcmp(buffer, "WIN") == 0) {
            printf("[+] You win!\n");
            break;
        } else if (strcmp(buffer, "LOSE") == 0) {
            printf("[+] You lose!\n");
            break;
        } else {
           printf("[+] Server replied: %s \n", buffer); 
        }

        // escape this loop, if the server sends message "quit"
        if (!bcmp(buffer, "quit", 4))
            break;
        // if server send ready signal, then we can start playing
        ready_to_play++;
    }

    pthread_exit(NULL);
    return 0;
}