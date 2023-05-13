#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>
#include<netdb.h>
#include<netinet/in.h>

void servicePlayers(int client1, int client2, int client3, int client4);

int main(int argc, char *argv[]) {
    char *myTime;
    int sd, client1, client2, client3, client4, portNumber;
    socklen_t len;
    struct sockaddr_in servAdd;
    int points;
    int32_t conv_points;
    int size = sizeof(conv_points);
    
    if (argc != 2) {
        fprintf(stderr,"Call model: %s <Port#>\n",argv[0]);
        exit(0);
    }
    
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "[-] Could not create socket\n");
        exit(1);
    } else {
        fprintf(stderr, "[+] Socket Created\n");
    }
    
    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    sscanf(argv[1], "%d", &portNumber);
    servAdd.sin_port = htons((uint16_t)portNumber);
    bind(sd, (struct sockaddr *) &servAdd,sizeof(servAdd));
    
    if (listen(sd, 6) == 0) {
        printf("[+] Listening...\n");
    } else {
        printf("[-] Error in binding.\n");
    }
    
    while (1) {
        client1 = accept(sd, (struct sockaddr*)NULL, NULL);
        write(client1, "Waiting for other players to join ...", 100);
        client2 = accept(sd, (struct sockaddr*)NULL, NULL);
        write(client1, "Player 2 joined ... Waiting for Player 3 and Player 4 to join ...", 100);
        write(client2, "Player 1 already joined ... Waiting for Player 3 and Player 4 to join ...", 100);
        client3 = accept(sd, (struct sockaddr*)NULL, NULL);
        write(client1, "Player 3 joined ... Waiting for Player 4 to join ...", 100);
        write(client2, "Player 3 joined ... Waiting for Player 4 to join ...", 100);
        write(client3, "You are Player 3 ... Waiting for Player 4 to join ...", 100);
        client4 = accept(sd, (struct sockaddr*)NULL, NULL);
        write(client1, "Player 4 already joined ... Game is Starting ...", 100);
        write(client2, "Player 4 already joined ... Game is Starting ...", 100);
        write(client3, "Player 4 already joined ... Game is Starting ...", 100);
        write(client4, "You are player 4 ... Game is Starting ...", 100);
        
        printf("[+] Got a game request\n");
        
        if (fork() == 0)
            servicePlayers(client1, client2, client3, client4);
    }
}

void servicePlayers(int client1, int client2, int client3, int client4) {
    int c1_points = 0, c2_points = 0, c3_points = 0, c4_points = 0;
    int32_t c1_conv_points, c2_conv_points, c3_conv_points, c4_conv_points;
    int size = sizeof(int32_t);
    char buf[100];
    
    while (1) {
        sleep(1);
        write(client1, "You can now play", 100);
        if (read(client1, &c1_conv_points, size) < 0) {
            printf("Read Error from Player 1");
        }
        c1_points += ntohl(c1_conv_points);
        snprintf(buf, 100, "Your Score is :: %d \nOpponents Scores \nPlayer 2 :: %d \nPlayer 3 :: %d \nPlayer 4 :: %d\n\n", c1_points, c2_points, c3_points, c4_points);
        write(client1, buf, 100);
        
        if (c1_points >= 100) {
            write(client1, "Game over: You won the game", 100);
            write(client2, "Game over: You lost the game and Player 1 won the game", 100);
            write(client3, "Game over: You lost the game and Player 1 won the game", 100);
            write(client4, "Game over: You lost the game and Player 1 won the game", 100);
            break;
        }
        
        sleep(2);
        write(client2, "You can now play", 100);
        if (read(client2, &c2_conv_points, size) < 0) {
            printf("Read Error from Player 2");
        }
        c2_points += ntohl(c2_conv_points);
        snprintf(buf, 100, "Your Score is :: %d \nOpponents Scores \nPlayer 1 :: %d \nPlayer 3 :: %d \nPlayer 4 :: %d\n\n", c2_points, c1_points, c3_points, c4_points);
        write(client2, buf, 100);
        
        if (c2_points >= 100) {
            write(client2, "Game over: You won the game", 100);
            write(client1, "Game over: You lost the game and Player 2 won the game", 100);
            write(client3, "Game over: You lost the game and Player 2 won the game", 100);
            write(client4, "Game over: You lost the game and Player 2 won the game", 100);
            break;
        }
        
        sleep(2);
        write(client3, "You can now play", 100);
        if (read(client3, &c3_conv_points, size) < 0) {
            printf("Read Error from Player 3");
        }
        c3_points += ntohl(c3_conv_points);
        snprintf(buf, 100, "Your Score is :: %d \nOpponents Scores \nPlayer 1 :: %d \nPlayer 2 :: %d \nPlayer 4 :: %d\n\n", c3_points, c1_points, c2_points, c4_points);
        write(client3, buf, 100);
        
        if (c3_points >= 100) {
            write(client3, "Game over: You won the game", 100);
            write(client1, "Game over: You lost the game and Player 3 won the game", 100);
            write(client2, "Game over: You lost the game and Player 3 won the game", 100);
            write(client4, "Game over: You lost the game and Player 3 won the game", 100);
            break;
        }
        
        sleep(2);
        write(client4, "You can now play", 100);
        if (read(client4, &c4_conv_points, size) < 0) {
			printf("Read Error from Player 4");
		}
		c4_points += ntohl(c4_conv_points);
		snprintf(buf, 100, "Your Score is :: %d \nOpponents Scores \nPlayer 1 :: %d \nPlayer 2 :: %d \nPlayer 3 :: %d\n\n", c4_points, c1_points, c2_points, c3_points);
		write(client4, buf, 100);

		if (c4_points >= 100) {
			write(client4, "Game over: You won the game", 100);
			write(client1, "Game over: You lost the game and Player 4 won the game", 100);
			write(client2, "Game over: You lost the game and Player 4 won the game", 100);
			write(client3, "Game over: You lost the game and Player 4 won the game", 100);
			break;
		}
	}
	close(client1);
	close(client2);
	close(client3);
	close(client4);
}		