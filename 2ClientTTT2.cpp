#include <winsock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <tchar.h>
#include <string>

const int PORT = 2024;
#pragma comment(lib, "ws2_32.lib") // Winsock Library

void handleError(bool err, const char* msg) {
    if (!err)
        return;
    std::cout << msg; // << WSAGetLastError();
    exit(EXIT_FAILURE);
}

int main(int argc, char const* argv[]) {
    // Initialize winsock std::cout << "Initialising Winsock...\n";
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa);
    handleError(res != 0, "Failed. Error Code : ");

    char buffer[1024] = { 0 };
    SOCKET client_sock = socket(AF_INET, SOCK_STREAM, 0);
    handleError(client_sock == SOCKET_ERROR, "Could not create socket : ");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    InetPton(AF_INET, _T("192.168.0.103"), &servaddr.sin_addr.s_addr);

    int status = connect(client_sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
    handleError(status == SOCKET_ERROR, "Could not connect to server : ");
    int valread = recv(client_sock, buffer, 1024, 0);

    if (buffer[0] == '1') {
        std::cout << "  0 1 2\na _|_|_\nb _|_|_\nc  | | \n";
        std::cout << "Your turn:";
        memset(buffer, 0, sizeof(buffer));

    }
    else {
        std::cout << "  0 1 2\na _|_|_\nb _|_|_\nc  | | \n";
        std::cout << "Please, wait for your opponent\n";
        int valread = recv(client_sock, buffer, 1024, 0);
        if (valread <= 0) {
            std::cout << "Connection closed by server.\n";
        }
        system("cls");
        std::cout << buffer;
        std::cout << "\nYour turn:";

    }

    std::string message;

    while (true) {
        //std::cout << "Enter your move (e.g., 0a0 for position (0, 'a') for 'o', 1a1 for position (1, 'a') for 'x'): ";
        std::getline(std::cin, message);

        // Отправка сообщения на сервер
        send(client_sock, message.c_str(), message.length(), 0);
        std::cout << "Message sent\n";
        system("cls");
        // Получение ответа от сервера
        int valread = recv(client_sock, buffer, 1024, 0);
        if (valread <= 0) {
            std::cout << "Connection closed by server.\n";
            break;
        }
        // Вывод обновленного игрового поля или сообщения о победе
        std::cout << buffer << '\n';
        std::cout << "Please, wait for your opponent\n";
        valread = recv(client_sock, buffer, 1024, 0);
        if (valread <= 0) {
            std::cout << "Connection closed by server.\n";
            break;
        }
        system("cls");
        std::cout << buffer;
        std::cout << "\nYour turn:";
        memset(buffer, 0, sizeof(buffer)); // Очистка буфера для следующего сообщения
    }

    // Закрытие соединения closesocket(client_sock);
    WSACleanup();
    return 0;
}