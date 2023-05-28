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

    // while (1)
    // {
    //     if (read(server, message, sizeof(message)) < 0)
    //     {
    //         fprintf(stderr, "read() error\n");
    //         exit(0);
    //     }
    //     printf("%s\n", message);

    //     if (!strcmp(message, "You can now play"))
    //     {
    //         int l;
    //         fscanf(stdin, "%d", &l);
    //         dice = rand() + l % 6 + 1;
    //         printf("********** You got: %d **********\n", dice);
    //         point = htonl(dice);
    //         write(server, &point, size);
    //     }
    // }
    exit(0);
}

void *handleServer(void* args) {
    int server = *(int*)args;

    while(1) {

    }
}