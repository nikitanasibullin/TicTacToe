#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <time.h>
#include <sstream>

#pragma comment(lib,"ws2_32.lib") //Winsock Library
int PORT = 2024;

const char to_TT(int x) {  //для вывода таблицы
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

const char to_TTplus(int x) { //для вывода нижней строчки таблицы
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

void logTime(std::ofstream& fout) {
    std::time_t now = std::time(0);
    std::tm localTime;
    localtime_s(&localTime, &now); // Используем localtime_s для получения локального времени
    fout << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << " - ";
}

void AnalyseTicTac(std::vector<int>& TicTac, std::string& rule, std::string& output, int& correct, std::ofstream& fout, int turn) {
    std::string sep = "|";
    if (rule[0] == 'b') { //если от клиента пришло "bad" => у него вышло время
        output = "  0 1 2\na ";
        output += to_TT(TicTac[0]) + sep + to_TT(TicTac[1]) + sep + to_TT(TicTac[2]);
        output += "\nb ";
        output += to_TT(TicTac[3]) + sep + to_TT(TicTac[4]) + sep + to_TT(TicTac[5]);
        output += "\nc ";
        output += to_TTplus(TicTac[6]) + sep + to_TTplus(TicTac[7]) + sep + to_TTplus(TicTac[8]);
        output = output + "\n" + "You win! Time is over!";
        correct = 2; //время истекло
        return;
    }

    int pos; //вычисляем позицию текущего хода в таблице
    int x = (rule[0] - '0');
    int y = (rule[1] - 'a');
    pos = 3 * y + x;

    if (TicTac[pos] == -1) {
        TicTac[pos] = (rule[2] == '0') ? 0 : 1;
    }
    else {
        correct = 3;
    }

    if (rule[0] != '0' && rule[0] != '1' && rule[0] != '2' || rule[1] != 'a' && rule[1] != 'b' && rule[1] != 'c') {
        correct = 0;
        output = "Please, try again";
        return;
    }

    //создание игрового поля
    output = "  0 1 2\na ";
    output += to_TT(TicTac[0]) + sep + to_TT(TicTac[1]) + sep + to_TT(TicTac[2]);
    output += "\nb ";
    output += to_TT(TicTac[3]) + sep + to_TT(TicTac[4]) + sep + to_TT(TicTac[5]);
    output += "\nc ";
    output += to_TTplus(TicTac[6]) + sep + to_TTplus(TicTac[7]) + sep + to_TTplus(TicTac[8]);

    // Проверка на выигрыш
    if (TicTac[0] == TicTac[1] && TicTac[1] == TicTac[2] && TicTac[0] != -1 ||
        TicTac[3] == TicTac[4] && TicTac[4] == TicTac[5] && TicTac[3] != -1 ||
        TicTac[6] == TicTac[7] && TicTac[7] == TicTac[8] && TicTac[6] != -1 ||
        TicTac[0] == TicTac[3] && TicTac[3] == TicTac[6] && TicTac[0] != -1 ||
        TicTac[1] == TicTac[4] && TicTac[4] == TicTac[7] && TicTac[1] != -1 ||
        TicTac[2] == TicTac[5] && TicTac[5] == TicTac[8] && TicTac[2] != -1 ||
        TicTac[0] == TicTac[4] && TicTac[4] == TicTac[8] && TicTac[0] != -1 ||
        TicTac[2] == TicTac[4] && TicTac[4] == TicTac[6] && TicTac[2] != -1) {
        output += "\nThe Game is over.";
        fout << "Player " << (turn + 1) << " wins!\n";
    }
    if (TicTac[0] != -1 and TicTac[1] != -1 and TicTac[2] != -1 and TicTac[3] != -1 and TicTac[4] != -1 and TicTac[5] != -1
        and TicTac[6] != -1 and TicTac[7] != -1 and TicTac[8] != -1) {
        output += "\nThe Game is over. The draw";
    }
}

void handleError(bool err, const char* msg, std::ofstream& fout) {
    if (!err)
        return;
    fout << msg << WSAGetLastError() << "\n";
    exit(EXIT_FAILURE);
}

int main(int argc, char const* argv[]) {

    srand(time(0));
    if (argc < 2) { return -1; }
    std::ifstream infile(argv[1]); // Открываем файл для чтения
    std::string line;
    std::string PASSWORD;
    int time = 0;

    // Проверяем, что файл успешно открыт
    if (!infile.is_open()) {
        std::cerr << "Cannot find the configuration file!" << std::endl;
        return 1;
    }

    // Читаем файл построчно
    for (size_t i = 0; i < 2;++i) {
        std::getline(infile, line);
        std::istringstream iss(line);
        std::string key, equal_sign;

        // Разделяем строку на ключ и значение
        if (iss >> key >> equal_sign) {
            if (equal_sign == "=") {
                if (key == "password") {
                    iss >> PASSWORD; // Считываем пароль
                }
                else if (key == "time") {
                    iss >> time; // Считываем время
                }
            }
        }
    }
    //PORT root
    std::string strPort;
    std::getline(infile, strPort);
    PORT = std::stoi(strPort);
    std::string logRoot;
    std::getline(infile, logRoot);

    infile.close(); // Закрываем файл

    std::ofstream fout;
    fout.open(logRoot);
    if (!fout) {
        std::cout << "Cannot open the file" << std::endl;
        return -1;
    }
    //std::cout << "Now data is:\t" + std::to_string(PORT) + '\t' + logRoot << '\n';
    // Initialize winsock
    logTime(fout);
    fout << "Initialising Winsock...\n";
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa);
    handleError(res != 0, "Failed. Error Code: ", fout);

    int valread;
    int opt = 1;
    const char* hello = "hello";
    char buffer[1024] = { 0 };
    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    handleError(server_sock == SOCKET_ERROR, "Could not create socket: ", fout);

    fout << "Server socket created.\n";

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    res = bind(server_sock, (struct sockaddr*)&address, sizeof(address));
    handleError(res == SOCKET_ERROR, "Bind failed with error code: ", fout);
    fout << "Bind done\n";

    std::vector<int> tictac{-1, -1, -1, -1, -1, -1, -1, -1, -1}; //

    res = listen(server_sock, 3);
    if (res < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    SOCKET new_socket1;
    SOCKET new_socket2;

    if (rand() % 2 == 0) { // случайно выбираем первого игрока
        new_socket1 = accept(server_sock, (struct sockaddr*)&address, &addrlen);
        logTime(fout);
        fout << "First player connected.\n";
        new_socket2 = accept(server_sock, (struct sockaddr*)&address, &addrlen);
        logTime(fout);
        fout << "Second player connected.\n";
    }
    else {
        new_socket2 = accept(server_sock, (struct sockaddr*)&address, &addrlen);
        logTime(fout);
        fout << "First player connected.\n";
        new_socket1 = accept(server_sock, (struct sockaddr*)&address, &addrlen);
        logTime(fout);
        fout << "Second player connected.\n";
    }
    handleError(new_socket1 == SOCKET_ERROR, "Accept failed with error code: ", fout);
    handleError(new_socket2 == SOCKET_ERROR, "Accept failed with error code: ", fout);

    // даем понять клиентам, кто из них первый, кто второй:
    std::string output = "1"; // первому отправляем единицу
    send(new_socket1, output.c_str(), output.size(), 0);
    output = "0"; // второму отправляем ноль.
    send(new_socket2, output.c_str(), output.size(), 0);

    bool correct_pass = true;
    recv(new_socket1, buffer, 1024, 0); // сначала принимаем Имя и пароль от первого пользователя
    std::istringstream inf1(buffer); // Создаем поток из строки
    std::string name;
    std::string password;

    // Считываем значения из потока
    inf1 >> name >> password;
    if (password != PASSWORD or !(name[0]>='A' and name[0]<='Z')) {
        correct_pass = false;
    }
    memset(buffer, 0, sizeof(buffer));
    send(new_socket2, output.c_str(), output.size(), 0); // второму пользователю отправляем ноль
    recv(new_socket2, buffer, 1024, 0); // принимаем пароль и имя от второго
    std::istringstream inf2(buffer);
    inf2 >> name >> password;
    if (password != PASSWORD or !(name[0] >= 'A' and name[0] <= 'Z')) {
        correct_pass = false;
    }
    memset(buffer, 0, sizeof(buffer));

    if (!correct_pass) { // если хоть у кого-то пароль не совпал или некорректное имя
        send(new_socket1, output.c_str(), output.size(), 0);
        send(new_socket2, output.c_str(), output.size(), 0);
        return 0;
    }
    else {
        output = "1"+std::to_string(time); // Если всё верно, то отправляем единичку
        send(new_socket1, output.c_str(), output.size(), 0);
        send(new_socket2, output.c_str(), output.size(), 0);
    }

    int turn = 0; // 0 для игрока 1 (o), 1 для игрока 2 (x)

    while (true) {
        output.clear();
        int correct = 1;
        SOCKET current_socket = (turn == 0) ? new_socket1 : new_socket2; // выбираем текущий сокет в зависимости от того, чей ход

        valread = recv(current_socket, buffer, 1024, 0);
        if (valread < 0) {
            perror("recv");
            break;
        }
        std::string rule(buffer);
        logTime(fout);
        fout << "Player " << (turn + 1) << " made a move: " << rule << "\n";

        rule += std::to_string(turn);
        AnalyseTicTac(tictac, rule, output, correct, fout, turn); // в output записана текущее игровое поле

        if (correct == 3) { // код ошибки если игрок сходил в непустую клетку
            output += "\nWrong step";
            logTime(fout);
            fout << "Player " << (turn + 1) << " made an invalid move: " << rule << "\n";
        }

        send(new_socket1, output.c_str(), output.size(), 0);
        send(new_socket2, output.c_str(), output.size(), 0);

        // Проверяем корректность хода
        if (correct == 0) {
            send(current_socket, "Invalid move. The game is over!", strlen("Invalid move. The game is over!"), 0);
            continue;
        }

        turn = 1 - turn; // Меняем ход

        if (output.find("over") != std::string::npos) { // проверяем, закончилась ли игра
            logTime(fout);
            fout << "Game over.\n";
            break;
        }
    }

    // Закрываем сокеты
    closesocket(new_socket1);
    closesocket(new_socket2);
    closesocket(server_sock);
    WSACleanup();
    fout.close();
    return 0;
}
