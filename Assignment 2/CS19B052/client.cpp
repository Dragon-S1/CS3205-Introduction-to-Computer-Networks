#include "bits/stdc++.h"
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define IP "127.0.0.1"

using namespace std;

// Board variables
int Board[3][3];
int filled = 0;
int sockfd = 0;
int id = 0;

// Function to draw the board
void drawBoard(){
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			if(Board[i][j] == 0)
				printf(" - ");
			else if(Board[i][j] == 1)
				printf(" O ");
			else if(Board[i][j] == 2)
				printf(" X ");
			if(j < 2)
				printf("|");
		}
		printf("\n");
	}
}

// Function to check the win condition
int checkBoard(){
	for(int i=0; i<3; i++)
		if(Board[i][0] != 0 && Board[i][0] == Board[i][1] && Board[i][1] == Board[i][2] && Board[i][0] == Board[i][2])
			return Board[i][0];
	for(int i=0; i<3; i++)
		if(Board[0][i] != 0 && Board[0][i] == Board[1][i] && Board[1][i] == Board[2][i] && Board[0][i] == Board[1][i])
			return Board[0][i];
	if(Board[0][0] != 0 && Board[0][0] == Board[1][1] && Board[1][1] == Board[2][2] && Board[0][0] == Board[2][2])
		return Board[0][0];
	if(Board[2][0] != 0 && Board[2][0] == Board[1][1] && Board[1][1] == Board[0][2] && Board[0][2] == Board[2][0])
		return Board[2][0];
	return 0;
}

// Function to check the validity of input and write to server
void soc_write(){
	char buff[MAX];
	int n;
	bzero(buff, sizeof(buff));
	printf("Enter <ROW COL> for placing your mark: ");
	n = 0;
	while(((buff[n++] = getchar()) != '\n'))
		;
	if(buff[0] == '\n')
		soc_write();
	if((buff[0] >= '1' && buff[0] <= '3') && (buff[2] >= '1' && buff[2] <= '3') && buff[1] == ' ' && n==4){
		int row = buff[0] - 48 - 1;
		int col = buff[2] - 48 - 1;
		if(Board[row][col] == 0){
			Board[row][col] = id;
			write(sockfd, buff, sizeof(buff));
			filled++;
			drawBoard();
			if(filled != 9)
				printf("Wait for opponent's turn\n");
		}
		else{
			printf("POSITION ALREADY FILLED\n");
			soc_write();
		}
	}
	else{
		printf("INVALID FORMAT\nCORRECT FORMAT: 'ROW COL'\n");
		soc_write();
	}
}

// Function to read the data from the server and return the needed data
char* soc_read(){
	char buff[MAX];
	int row, col;
	char* ret = (char*)malloc(3 * sizeof(char));
	bzero(buff, sizeof(buff));
	read(sockfd, buff, sizeof(buff));
	row = buff[0] - 48;
	col = buff[2] - 48;
	sprintf(ret,"%d %d",row,col);
	return ret;
}

// Function the read the data from sever and return the notification char
char id_read(){
	char buff[MAX];
	bzero(buff, sizeof(buff));
	read(sockfd, buff, sizeof(buff));
	return buff[0];
}

// Function to play the game
void startGame(){
	// Empty the board
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			Board[i][j] = 0;
		}
	}
	filled = 0;
	drawBoard();

	if(id == 2)
		printf("Wait for opponent's turn\n");

	// Write and read data according to the player id
	while(filled != 9){
		char* data = (char*)malloc(3 * sizeof(char));
		int row,col;
		// For player id 1
		if(id == 1){
			if(checkBoard() != 0 || filled == 9)
				break;
			soc_write();
			if(checkBoard() != 0 || filled == 9)
				break;
			data = soc_read();

			// Check if the opponent disconnected
			if(data[0] == 'q' || data[1] != ' '){
				printf("Opponent disconnected\n");
				free(data);
				return;
			}

			// Fill the Board
			row = data[0]-48;
			col = data[2]-48;
			Board[row][col] = 2;
			filled++;
			drawBoard();
		}
		// For player id 2
		else{
			data = soc_read();

			// Check if the opponent disconnected
			if(data[0] == 'q' || data[1] != ' '){
				printf("Opponent disconnected\n");
				free(data);
				return;
			}

			// Fill the Board
			row = data[0]-48;
			col = data[2]-48;
			Board[row][col] = 1;
			filled++;
			drawBoard();
			if(checkBoard() != 0 || filled == 9)
				break;
			soc_write();
			if(checkBoard() != 0 || filled == 9)
				break;
		}
		free(data);
	}
	
	// Check the final status of board
	int sym = checkBoard();
	if(sym == id)
		printf("Congrutalutions you WON!\n");
	else if(sym != 0)
		printf("Your opponent won\n");

	if(sym == 0)
		printf("DRAW!\n");

	// Get input for replay of match
	printf("Do you want to replay?(YES/NO): ");
	char buff[MAX];
	bzero(buff, sizeof(buff));
	
	int n = 0;
	while(((buff[n++] = getchar()) != '\n'))
		;

	// Write reply to the server
	write(sockfd, buff, sizeof(buff));

	// Get replay status from server
	char res = id_read();

	// Replay or exit the game
	if(res == '0')
		startGame();
	else if(res != id+48){
		printf("Opponent refused to play again\n");
	}
}

  
// Driver funtion
int main()
{
	int connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
			printf("socket creation failed...\n");
			exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));
	
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP);
	servaddr.sin_port = htons(PORT);
	
	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}

	// Get the player id from server
	char id_char = id_read();

	if(id_char == '1'){
		id = 1;
		printf("Connected to the game server. Your player ID is 1. Waiting for a partner to join...\n");
	}
	else{
		id = 2;
		printf("Connected to the game server. Your player ID is 2. Your partner's ID is 1. Your symbol is 'X'\n");
	}

	// Get notification data when opponent joins
	id_char = id_read();
	if(id_char == '0'){
		if(id == 1){
			printf("Your partner's ID is 2. Your symbol is 'O'.\n");
			printf("Starting the game...\n");

		}
		else if(id == 2){
			printf("Starting the game...\n");
		}
	}

	// start the game with player id
	startGame();
	
	// close the socket
	close(sockfd);
}