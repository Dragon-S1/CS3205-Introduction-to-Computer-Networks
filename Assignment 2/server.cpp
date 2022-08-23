#include "bits/stdc++.h"
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#define MAX 100
#define PORT 8080
#define SA struct sockaddr
#define MAX_GAMES 100

using namespace std;

// Board variables
int Board[3][3];
int filled = 0;

// Function to draw the board
void drawBoard(int board[3][3]){
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			if(board[i][j] == 0)
				printf(" - ");
			else if(board[i][j] == 1)
				printf(" O ");
			else if(board[i][j] == 2)
				printf(" X ");
			if(j < 2)
				printf("|");
		}
		printf("\n");
	}
}

// Function to check the win condition
int checkBoard(int board[3][3]){
	for(int i=0; i<3; i++)
		if(board[i][0] != 0 && board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] == board[i][2])
			return board[i][0];
	for(int i=0; i<3; i++)
		if(board[0][i] != 0 && board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] == board[1][i])
			return board[0][i];
	if(board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] == board[2][2])
		return board[0][0];
	if(board[2][0] != 0 && board[2][0] == board[1][1] && board[1][1] == board[0][2] && board[0][2] == board[2][0])
		return board[2][0];
	return 0;
}

// Function to send and receive data from both player for tictactoe
void startGame(int connfd1, int connfd2, int game_id, int p1, int p2){
	// Get the game start time
	auto start = std::chrono::system_clock::now();
  std::time_t game_start = std::chrono::system_clock::to_time_t(start);
	
	// File output object
	ofstream output;
	char* filename = (char*)malloc(MAX * sizeof(char));
	sprintf(filename,"%d.log",game_id);										// Put the filename as <game_id>.log
	output.open(filename);
	free(filename);

	// Output the game start time and game detail in the log file
	output<< "Game " <<  game_id << " started between Player " << p1 << " and Player " << p2 << endl;
	output<< "Started at: "<< std::ctime(&game_start);
	output << endl;

	// Empty the board
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			Board[i][j] = 0;
		}
	}
	filled = 0;

	int turn = 1;
	int timeout = 0;
	// Loop to send and receive data from players
	while(checkBoard(Board) == 0 && filled != 9){
		char buff[MAX];
		char* data = (char*)malloc(3 * sizeof(char));
		int row,col;
		bzero(buff, sizeof(buff));

		if(turn == 1){
			// Read data from player 1
			read(connfd1, buff, sizeof(buff));
			if(buff[0] == 't'){
				timeout = 1;
				break;
			}
			row = buff[0] - 48 - 1;
			col = buff[2] - 48 - 1;

			// Check if the player has disconnected
			if(row<0 || col<0){
				printf("Player %d disconnected\n", p1);
				output << "Player " << p1 << " disconnected, Ending Game..." << endl;

				// Send data to notify other player
				sprintf(data, "q");
				write(connfd2, data, sizeof(data));

				// Close the connections
				close(connfd1);
				close(connfd2);
				free(data);

				// Get the end time
				auto end = std::chrono::system_clock::now();
				std::time_t game_end = std::chrono::system_clock::to_time_t(end);
				// Get the interval of match
				std::chrono::duration<double> elapsed_seconds = end-start;

				// Output the end time and interval to the log file
				output << endl;
				output << "Game ended at: " << std::ctime(&game_end);
				output << "Total Duration: " << elapsed_seconds.count() << "s";

				// Close the file
				output.close();
				return;
			}

			// Output the player move into log file
			output << "Player 1: " << row+1 << " " << col+1 << endl;

			// Fill the board
			Board[row][col] = 1;
			filled++;

			// Send the data from player 1 to player 2
			sprintf(data, "%d %d\n", row, col);
			write(connfd2, data, sizeof(data));
			turn = 2;
		}
		else{
			// Read data from player 2
			read(connfd2, buff, sizeof(buff));
			if(buff[0] == 't'){
				timeout = 2;
				break;
			}
			row = buff[0] - 48 - 1;
			col = buff[2] - 48 - 1;

			// Check if the player has disconnected
			if(row<0 || col<0){
				printf("Player %d disconnected\n", p2);
				output << "Player " << p2 << " disconnected, Ending Game..." << endl;

				// Send data to notify other player
				sprintf(data, "q");
				write(connfd1, data, sizeof(data));

				// Close the connections
				close(connfd1);
				close(connfd2);
				free(data);

				// Get the end time
				auto end = std::chrono::system_clock::now();
  			std::time_t game_end = std::chrono::system_clock::to_time_t(end);
				// Get the interval of match
  			std::chrono::duration<double> elapsed_seconds = end-start;

				// Output the end time and interval to the log file
				output << endl;
				output << "Game ended at: " << std::ctime(&game_end);
				output << "Total Duration: " << elapsed_seconds.count() << "s";

				// Close the file
				output.close();
				return;
			}

			// Output the player move into log file
			output << "Player 2: " << row+1 << " " << col+1 << endl;

			// Fill the board
			Board[row][col] = 2;
			filled++;

			// Send the data from player 1 to player 2
			sprintf(data, "%d %d", row, col);
			write(connfd1, data, sizeof(data));
			turn = 1;
		}
		free(data);
	}
	if(timeout == 1){
		char buff[MAX];
		bzero(buff, sizeof(buff));
		sprintf(buff, "t");
		write(connfd2, buff, sizeof(buff));
	}
	if(timeout == 2){
		char buff[MAX];
		bzero(buff, sizeof(buff));
		sprintf(buff, "t");
		write(connfd1, buff, sizeof(buff));
	}
	
	if(!timeout){
		// Output the result of match
		output << endl;
		int winner = checkBoard(Board);
		if(winner == 1)
			output << "RESULT: Player " << p1 << " is the WINNER!!" << endl;
		else if(winner == 2)
			output << "RESULT: Player " << p2 << " is the WINNER!!" << endl;
		else
			output << "RESULT: DRAW!" << endl;
	}

	// Get the reply from both players regarding replaying the match
	char buff1[MAX];
	char buff2[MAX];
	bzero(buff1, sizeof(buff1));
	bzero(buff2, sizeof(buff2));

	read(connfd1, buff1, sizeof(buff1));
	read(connfd2, buff2, sizeof(buff2));

	char res[1];
	int restart = 0;

	// Check the reply from both players
	output << endl;
	if((strncmp("YES", buff1, 3) == 0) && (strncmp("YES", buff2, 3) == 0)){
		res[0] = '0';
		restart = 1;
		output << "Both Player choose to play again, new game will be started with id: " << game_id+MAX_GAMES << endl;
	}
	else if((strncmp("YES", buff1, 3) != 0) && (strncmp("YES", buff2, 3) == 0)){
		res[0] = '1';
		output << "Player " << p1 << " choose to not play again, Ending Game..." << endl;
	}
	else if((strncmp("YES", buff1, 3) == 0) && (strncmp("YES", buff2, 3) != 0)){
		res[0] = '2';
		output << "Player " << p2 << " choose to not play again, Ending Game..." << endl;
	}
	else{
		res[0] = '3';
			output << "Both Player choose to not play again, Ending Game..." << endl;
	}
	
	// Send the data to notify both players if the match is to be replayed
	write(connfd1, res, sizeof(res));
	write(connfd2, res, sizeof(res));

	// Get the end time
	auto end = std::chrono::system_clock::now();
	std::time_t game_end = std::chrono::system_clock::to_time_t(end);
	// Get the interval of match
	std::chrono::duration<double> elapsed_seconds = end-start;

	// Output the end time and interval to the log file
	output << endl;
	output << "Game ended at: " << std::ctime(&game_end);
	output << "Total Duration: " << elapsed_seconds.count() << "s";
	if(restart == 1)
		startGame(connfd1, connfd2, game_id + MAX_GAMES, p1, p2);

	// Close the connections
	close(connfd1);
	close(connfd2);

	// Close the file
	output.close();
}

// Create socket
void create_socket(int* sockfd, SA* cli, socklen_t* len, int max_conn){
	struct sockaddr_in servaddr;

	// socket create and verification
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	
	// Binding newly created socket to given IP and verification
	if ((bind(*sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	
	// Now server is ready to listen and verification
	if ((listen(*sockfd, max_conn)) != 0) {
		printf("Game server failes to start\n");
		exit(0);
	}
	else
		printf("Game server started. Waiting for players.\n");
	*len = sizeof(*cli);
}

   

// Driver function
int main(){
	signal(SIGPIPE, SIG_IGN);
	sigignore(SIGPIPE);
	int sockfd;
	socklen_t len;
	struct sockaddr_in cli;

	// Create Socket
	create_socket(&sockfd, (SA*)&cli, &len, 2*MAX_GAMES);

	// Wait for 2 player connections and make thread for each game
	int game_id = 0;
	int num_players = 1;
	while(1){
		// Accept the connection from first player
		int connfd1 = accept(sockfd, (SA*)&cli, &len);
		if (connfd1 < 0) {
				printf("server accept failed...\n");
				exit(0);
		}
		else
				printf("Player %d added!\n", num_players);

		int p1 = num_players;
		num_players++;
		char con_id[1];

		// Send player id to first player
		con_id[0] = '1';
		write(connfd1, con_id, sizeof(con_id));

		// Accept the connection from second player
		int connfd2 = accept(sockfd, (SA*)&cli, &len);
		if (connfd2< 0) {
				printf("server accept failed...\n");
				exit(0);
		}
		else
				printf("Player %d added!\n", num_players);

		// Send player id to second player
		int p2 = num_players;
		num_players++;
		con_id[0] = '2';
		write(connfd2, con_id, sizeof(con_id));

		// Send data to the players to notify them to start the game
		con_id[0] = '0';
		write(connfd1, con_id, sizeof(con_id));
		write(connfd2, con_id, sizeof(con_id));

		// Create the thread for a game
		thread game(startGame,connfd1, connfd2, game_id, p1, p2);

		// Detach the thread
		game.detach();

		// Increase the game id counter
		game_id++;
	}

	// Close the server socket
	close(sockfd);
}