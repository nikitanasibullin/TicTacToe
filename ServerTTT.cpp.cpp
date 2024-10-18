// Server side C/C++ program to demonstrate Socket
// programming
#include<winsock2.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>

#pragma comment(lib,"ws2_32.lib") //Winsock Library
const int PORT = 2024;

const char to_TT(int x) {
	if (x == 1) {
		return 'x';
	}
	else if (x == 0) {
		return 'o';
	}
	else {
		return '_';
	}
}
const char to_TTplus(int x) {
	if (x == 1) {
		return 'x';
	}
	else if (x == 0) {
		return 'o';
	}
	else {
		return ' ';
	}
}
void AnalyseTicTac(std::vector<int>& TicTac,std::string& rule, std::string& output, int& correct ) {
	int pos;
	int x = (rule[0]-'0');
	int y = (rule[1] - 'a');
	pos = 3*y + x;
	if (TicTac[pos]==-1) {
		if (rule[2] == '0') {
			TicTac[pos] = 0;
		}
		else {
			TicTac[pos] = 1;
		}
	}
	else {
		correct = 0;
		output = "Please, try again";
		return;
	}
	std::string sep = "|";
	output = "  0 1 2\na ";
	output += to_TT(TicTac[0]) + sep + to_TT(TicTac[1]) + sep + to_TT(TicTac[2]);
	output += "\nb ";
	output += to_TT(TicTac[3]) + sep + to_TT(TicTac[4]) + sep + to_TT(TicTac[5]);
	output += "\nc ";
	output += to_TTplus(TicTac[6]) + sep + to_TTplus(TicTac[7]) + sep + to_TTplus(TicTac[8]);
	correct = 1;

	if (TicTac[0] == TicTac[1] and TicTac[1] == TicTac[2] and TicTac[0] != -1
		or TicTac[3] == TicTac[4] and TicTac[4] == TicTac[5] and TicTac[3] != -1
		or TicTac[6] == TicTac[7] and TicTac[7] == TicTac[8] and TicTac[6] != -1
		or TicTac[0] == TicTac[3] and TicTac[3] == TicTac[6] and TicTac[0] != -1
		or TicTac[1] == TicTac[4] and TicTac[4] == TicTac[7] and TicTac[1] != -1
		or TicTac[2] == TicTac[5] and TicTac[5] == TicTac[8] and TicTac[2] != -1
		or TicTac[0] == TicTac[4] and TicTac[4] == TicTac[8] and TicTac[0] != -1
		or TicTac[2] == TicTac[4] and TicTac[4] == TicTac[6] and TicTac[2] != -1)
	{
		output =output+ "\n" +to_TT(rule[2]-'0')+ " won!!";
	}

}



void handleError(bool err, const char* msg, std::ofstream & fout)
{
	if (!err)
		return;
	fout << msg << WSAGetLastError();
	exit(EXIT_FAILURE);
}

int main(int argc, char const* argv[])
{
	srand(time(0));
	std::ofstream fout;
	fout.open("log.txt");
	if (!fout) {
		std::cout << "can not open the file" << std::endl;
		return -1;
	}
	//Initialize winsock
	fout << "Initialising Winsock...\n";
	WSADATA wsa;
	int res = WSAStartup(MAKEWORD(2, 2), &wsa);
	handleError(res != 0, "Failed. Error Code : ", fout);

	int valread;
	int opt = 1;
	const char* hello = "hello";
	char buffer[1024] = { 0 };
	SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
	// Creating socket file descriptor
	handleError(server_sock == SOCKET_ERROR, "Could not create socket : ", fout);

	fout << "Server socket created.\n";

	struct sockaddr_in address;
	int addrlen = sizeof(address);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	res = bind(
		server_sock,
		(struct sockaddr*)&address,
		sizeof(address)
	);
	handleError(res == SOCKET_ERROR, "Bind failed with error code : ", fout);
	fout << "Bind done\n";
	//std::cout << "  0 1 2\na _|x|_\nb _|_|_\nc  |o| ";

	std::vector<int> tictac{-1, -1, -1, -1, -1, -1, -1, -1, -1};
	

	res = listen(server_sock, 3);
	if (res < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	SOCKET new_socket1;
	SOCKET new_socket2;

	if (rand() % 2 == 0) { //случайно выбираем первого игрока
		new_socket1 = accept(
			server_sock,
			(struct sockaddr*)&address,
			&addrlen
		);

		new_socket2 = accept(
			server_sock,
			(struct sockaddr*)&address,
			&addrlen
		);
	}
	else {
		new_socket2 = accept(
			server_sock,
			(struct sockaddr*)&address,
			&addrlen
		);

		new_socket1 = accept(
			server_sock,
			(struct sockaddr*)&address,
			&addrlen
		);
	}
	
	handleError(new_socket1 == SOCKET_ERROR, "Accept failed with error code : ", fout);
	fout << "first socket Accepted\n";
	handleError(new_socket2 == SOCKET_ERROR, "Accept failed with error code : ", fout);
	fout << "second socket Accepted\n";
	std::string output = "1";
	send(new_socket1, output.c_str(), output.size(), 0);
	output = "0";
	send(new_socket2, output.c_str(), output.size(), 0);
	

	int turn = 0; // 0 for player 1 (o), 1 for player 2 (x)

	while (true) {
		std::string output;
		int correct = 1;
		SOCKET current_socket = (turn == 0) ? new_socket1 : new_socket2;

		// Receive message from the current player
		valread = recv(current_socket, buffer, 1024, 0);
		if (valread < 0) {
			perror("recv");
			break;
		}
		std::string rule(buffer);

		std::cout << "msg from client " << (turn + 1) << ": " << rule << '\n';
		rule += std::to_string(turn);
		AnalyseTicTac(tictac, rule, output, correct);
		send(new_socket1, output.c_str(), output.size(), 0);
		send(new_socket2, output.c_str(), output.size(), 0);

		// Check if the move was correct
		if (correct == 0) {
			send(current_socket, "Invalid move. Please try again.", strlen("Invalid move. Please try again."), 0);
			continue;
		}

		// Change turn
		turn = 1 - turn; // Switch between 0 and 1

		// Check if game is over (win or draw)
		if (output.find("won") != std::string::npos) {
			break; // Exit the loop if someone has won }
		}
	}
	// closing the connected socket
	closesocket(new_socket1);
	closesocket(new_socket2);
	// closing the listening socket
	closesocket(server_sock);
	WSACleanup();
	return 0;
}