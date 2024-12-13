#include <winsock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <tchar.h>
#include <string>
#include <thread>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <atomic>


int PORT = 2024;
LPCSTR IP = ("127.0.0.1"); //айпи сервера
std::atomic<bool> timeout_flag(false); // Флаги для индикации таймаута
std::atomic<bool> timeover_flag(false);

#pragma comment(lib, "ws2_32.lib")


void timerThread(SOCKET client_sock, int time) {
    std::this_thread::sleep_for(std::chrono::seconds(time));
    if (!timeout_flag) {
        std::string timeout_message = "bad";
        send(client_sock, timeout_message.c_str(), timeout_message.length(), 0);
        system("cls");
        std::cout << "Your time is over :(\nYou lose!"; //если выйдет время у нас, то отправляем серверу "bad"
        timeout_flag = true;                            //и закрываем сокеты
        timeover_flag = true;
        std::cin.ignore(1000, '\n');
        WSACleanup();
    }
}

void logTime(std::ofstream& fout) {
    std::time_t now = std::time(0);
    std::tm localTime;
    localtime_s(&localTime, &now); // Используем localtime_s для получения локального времени
    fout << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << " - ";
}

void handleError(bool err, const char* msg, std::ofstream& fout) {
    if (!err)
        return;
    logTime(fout);
    fout << msg << std::endl;
    std::cout << msg;
    exit(EXIT_FAILURE);
}

int main(int argc, char const* argv[]) {
    if (argc < 2) { return -1; };
    std::ifstream konfig(argv[1]);//Открываем конфигурационный файл
    if (!konfig) {
        std::cout << "Cannot open configuration file" << std::endl;
        return -1;
    }
    //IP PORT root
    std::string strIp;
    std::getline(konfig, strIp);
    IP = (strIp.c_str());
    std::string strPort;
    std::getline(konfig, strPort);
    PORT = std::stoi(strPort);
    std::string logRoot;
    std::getline(konfig, logRoot);
    konfig.close();
    std::ofstream fout(logRoot, std::ios::app); // Открываем лог файл в режиме добавления
    if (!fout) {
        std::cout << "Cannot open log file" << std::endl;
        logTime(fout);
        fout << "Cannot open log file" << std::endl;
        return -1;
    }

    //std::cout << "Now data is:\t"<<IP<<'\t' << std::to_string(PORT) <<'\t'+logRoot<<'\n';
    int time = 100;
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa);
    handleError(res != 0, "Failed. Error Code : ", fout);

    char buffer[1024] = { 0 };
    SOCKET client_sock = socket(AF_INET, SOCK_STREAM, 0);
    handleError(client_sock == SOCKET_ERROR, "Could not create socket : ", fout);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    InetPtonA(AF_INET, (IP), &servaddr.sin_addr.s_addr);

    int status = connect(client_sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
    handleError(status == SOCKET_ERROR, "Could not connect to server : ", fout);

    std::string password;
    int valread = recv(client_sock, buffer, 1024, 0);
    handleError(valread <= 0, "Connection closed by server.\n", fout);

    logTime(fout);
    fout << "Connected to server." << std::endl;

    if (buffer[0] == '1') {
        std::cout << "It's a game of tic-tac-toe!\nDuring your turn, choose a cell to go to : e.g.‘1a’." << std::endl;
        std::cout << "Remember that your move time is limited to ten seconds.\nIf you move to an occupied cell, you automatically lose.\nYou mark by \"o\"\n\nEnter a capitalised name in Latin letters and password with a space to start the game : ";
        std::getline(std::cin, password);
        logTime(fout);
        fout << "User  input: " << password << std::endl;
        send(client_sock, password.c_str(), password.length(), 0); //первый клиент сразу отправляет пароль

        system("cls");
        std::cout << "Please, wait for your opponent. The game will start soon!";

        memset(buffer, 0, sizeof(buffer)); // Очистка буфера для следующего сообщения
        valread = recv(client_sock, buffer, 1024, 0);//ждем единицы, если пароли совпали
        logTime(fout);
        fout << "User  output: " << buffer << std::endl;
        system("cls");
        handleError(valread <= 0, "Connection closed by server.\n", fout);
        if (buffer[0] != '1') {
            fout << "The password by one of the clients is wrong. Session is closed!";
            std::cout << "The password by one of the clients is wrong. Session is closed!";
            WSACleanup();
            return 0;
        }
        char* newBuffer = buffer + 1; // Сдвигаем указатель на следующий символ
        // Преобразуем оставшуюся часть строки в int
        time = std::atoi(newBuffer);

        std::cout << "  0 1 2\na _|_|_\nb _|_|_\nc  | | \n";
        std::cout << "Your turn:";
        memset(buffer, 0, sizeof(buffer));

    }
    else {
        std::cout << "It's a game of tic-tac-toe!\nDuring your turn, choose a cell to go to : e.g.‘1a’." << std::endl;
        std::cout << "Remember that your move time is limited to ten seconds.\nIf you move to an occupied cell, you automatically lose.\nYou mark by \"x\"\n\nEnter a capitalised name in Latin letters and password with a space to start the game : ";
        std::getline(std::cin, password);
        system("cls");
        std::cout << "Please, wait for your opponent. The game will start soon!";
        valread = recv(client_sock, buffer, 1024, 0); //ждем пока сервер нам скажет, что можно отправлять пароль
        logTime(fout);
        fout << "User  output: " << buffer << std::endl;
        handleError(valread <= 0, "Connection closed by server.\n", fout);
        system("cls");

        send(client_sock, password.c_str(), password.length(), 0); //отправляем пароль
        logTime(fout);
        fout << "User  input: " << password << std::endl;
        memset(buffer, 0, sizeof(buffer)); // Очистка буфера для следующего сообщения

        valread = recv(client_sock, buffer, 1024, 0);//ждем единицы, если пароли совпали
        handleError(valread <= 0, "Connection closed by server.\n", fout);
        logTime(fout);
        fout << "User  output: " << buffer << std::endl;

        if (buffer[0] != '1') { //если не совпали
            fout << "The password by one of the clients is wrong. Session is closed!";
            std::cout << "The password by one of the clients is wrong. Session is closed!";
            WSACleanup();
            return 0;
        }
        char* newBuffer = buffer + 1; // Сдвигаем указатель на следующий символ
        // Преобразуем оставшуюся часть строки в int
        time = std::atoi(newBuffer);
        memset(buffer, 0, sizeof(buffer)); // Очистка буфера для следующего сообщения

        std::cout << "  0 1 2\na _|_|_\nb _|_|_\nc  | | \n"; //пустая таблица
        std::cout << "Please, wait for your opponent\n";


        valread = recv(client_sock, buffer, 1024, 0);
        if (buffer[strlen(buffer) - 1] == '!') { //если пришел "!" на конце,значит у оппонента истекло время.
            system("cls");
            logTime(fout);
            fout << "game is over";
            std::cout << "Your opponent's time is over!\nYou win!\n";
            closesocket(client_sock);
            WSACleanup();
            return 0;
        }
        logTime(fout);
        fout << "User  output: " << buffer << std::endl;
        handleError(valread <= 0, "Connection closed by server.\n", fout);
        system("cls");
        std::cout << buffer;
        std::cout << "\nYour turn:";

    }

    std::string message;
    while (true) {
        timeout_flag = false; // Сброс флага таймаута после каждого кода
        // Запуск таймера в отдельном потоке
        std::thread timer(timerThread, client_sock, time);

        std::getline(std::cin, message);
        if (!timeover_flag) {
            timeout_flag = true;
            // Отправка сообщения на сервер
            while (message.size() != 2 or message[0] != '1' and message[0] != '2' and message[0] != '0' or message[1] != 'a' and message[1] != 'b' and message[1] != 'c') {
                std::cout << "Incorrect move!\nPlease, try again:";
                std::getline(std::cin, message);
            }

            send(client_sock, message.c_str(), message.length(), 0);
            logTime(fout);
            fout << "User  intput: " << message << std::endl;
            system("cls");

            // Получение текущего поля от сервера
            int valread = recv(client_sock, buffer, 1024, 0);
            handleError(valread <= 0, "Connection closed by server.\n", fout);
            logTime(fout);
            fout << "User  output: " << buffer << std::endl;


            // Вывод обновленного игрового поля или сообщения о победе
            std::cout << buffer << '\n';

            if (buffer[strlen(buffer) - 1] == '.') { //если точка пришла после нашего хода,значит мы выйграли
                logTime(fout);
                fout << "game is over";
                std::cout << "You win!\n";
                timer.join();
                closesocket(client_sock);
                WSACleanup();
                return 0;
            }
            else if (buffer[strlen(buffer) - 1] == 'p') { //пришло сооьщение о том, что в наш ход закрасили непустую клетку
                logTime(fout);
                fout << "game is over";
                std::cout << "You lose!\n";
                timer.join();
                closesocket(client_sock);
                WSACleanup();
                return 0;
            }
            else if (buffer[strlen(buffer) - 1] == 'w') { //пришло сообщение: ничья
                std::cout << buffer;
                timer.join();
                logTime(fout);
                fout << "game is over";
                closesocket(client_sock);
                WSACleanup();
                return 0;
            }

            std::cout << "Please, wait for your opponent\n";

            valread = recv(client_sock, buffer, 1024, 0);//принимаем ход оппонента
            handleError(valread <= 0, "Connection closed by server.\n", fout);
            logTime(fout);
            fout << "User  output: " << buffer << std::endl;

            system("cls");

            if (buffer[strlen(buffer) - 1] == '!') { //если пришел "!" на конце,значит время истекло у оппонента.
                timer.join();
                logTime(fout);
                fout << "game is over";
                std::cout << "Your opponent's time is over!\nYou win!\n";
                closesocket(client_sock);
                WSACleanup();
                return 0;
            }
            else if (buffer[strlen(buffer) - 1] == '.') { //если точка на конце пришла после хода соперника, то он выйграл.
                std::cout << buffer << '\n';
                timer.join();
                logTime(fout);
                fout << "game is over";
                std::cout << "You lose!\n";
                closesocket(client_sock);
                WSACleanup();
                return 0;
            }
            else if (buffer[strlen(buffer) - 1] == 'p') { //пришло сообщение о том, что в чужой ход закрасили непустую клетку
                std::cout << buffer;
                std::cout << "You win!\n";
                timer.join();
                logTime(fout);
                fout << "game is over";
                closesocket(client_sock);
                WSACleanup();
                return 0;
            }
            else if (buffer[strlen(buffer) - 1] == 'w') { //пришло сообщение: ничья
                std::cout << buffer;
                timer.join();
                logTime(fout);
                fout << "game is over";
                closesocket(client_sock);
                WSACleanup();
                return 0;
            }

            std::cout << buffer << std::endl;


            std::cout << "\nYour turn:";

            memset(buffer, 0, sizeof(buffer)); // Очистка буфера для следующего сообщения
        }
        else {
            timer.join();
            break;
        }


        // Ожидание завершения потока таймера
        timer.join();
    }

    //закрывае файлы 
    
    // Закрытие соединения 
    closesocket(client_sock);
    WSACleanup();
    return 0;
}
