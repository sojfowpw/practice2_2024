#include "parser.h"
#include "insert.h"
#include "delete.h"
#include "select.h"

#include "parser.cpp"
#include "insert.cpp"
#include "delete.cpp"
#include "select.cpp"

#include <iostream>
#include <sys/socket.h> // функции для работы с сокетами
#include <netinet/in.h> // структуры данных для портов
#include <unistd.h> // функции для работы с системными вызовами
#include <string.h>
#include <thread>
#include <sstream>

using namespace std;

enum class Commands { // существующие команды
    EXIT,
    INSERT,
    DELETE,
    SELECT,
    ERR
};

Commands stringToCommand(const string& cmd) { // определение команд
    istringstream iss(cmd); // поток ввода для обработки строки команды
    string word;
    iss >> word;
    if (word == "EXIT") {
        return Commands::EXIT;
    }
    else if (word == "INSERT") {
        return Commands::INSERT;
    }
    else if (word == "DELETE") {
        return Commands::DELETE;
    }
    else if (word == "SELECT") {
        return Commands::SELECT;
    }
    else {
        return Commands::ERR;
    }
}

int main() {
    tableJson tjs;
    parsing(tjs);
    cout << "\n\n";

    cout << "Загрузка сервера\n";
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // создание сокета для сервера
    // AF_INET сокет используется для работы с IPv4 - протокол передачи информации внутри сети интернет
    // SOCK_STREAM - сокет типа TCP
    // использование протокола по умолчанию для данного типа сокета
    if (serverSocket == -1) {
        cerr << "Не удалось создать сокет\n";
        return 1;
    }

    sockaddr_in serverAddress; // определение адреса сервера, тип данных для хранения адреса сокета
    serverAddress.sin_family = AF_INET; // семейство адресов IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY; // 32 битный IPv4
    serverAddress.sin_port = htons(7432); // преобразует номер порта 7432 из хостового порядка байтов в сетевой порядок байтов
    // привязываем сокет к указанному адресу и порту
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // (struct sockaddr*)&serverAddress - указатель на структуру sockaddr_in
        cerr << "Связь не удалась\n";
        close(serverSocket);
        return 1;
    }
    // прослушивание входящих соединений
    if (listen(serverSocket, 3) < 0) { // 3 максимальное количество соединений в очереди
        cerr << "Прослушивание не удалось\n";
        close(serverSocket);
        return 1;
    }
    cout << "Ожидание входящих соединений\n";

    while (true) { // принятие соединений
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress); // размер 
        int newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength); // принятие клиента
        if (newSocket < 0) {
            cerr << "Соединение не принято\n";
            close(serverSocket);
            continue;
        }
        cout << "Соединение принято\n";

        thread t([newSocket, &tjs] () { // новый поток для соединения
            char buffer[1024] = {0}; // буфер 1024 байта, инициализированный 0
            while (true) {
                int valread = read(newSocket, buffer, 1024); // чтение данных в буфер, valread - количество байт
                if (valread <= 0) {
                    cerr << "Клиент отсоединился\n";
                    break;
                }
                string command; // Преобразуем буфер в строку
                command = string(buffer, valread);
                cout << "Сообщение получено: " << command; // вывод сообщения клиента
                Commands cmd = stringToCommand(command); // обработка введённой команды
                switch (cmd) {
                    case Commands::EXIT: // выход
                        close(newSocket);
                        return; // Возвращаемся из лямбда-функции
                    case Commands::INSERT: // вставка
                        insert(command, tjs);
                        break;
                    case Commands::DELETE: // удаление
                        del(command, tjs);
                        break;
                    case Commands::SELECT: // выбор
                        select(command, tjs);
                        cout << "Select выполнен\n";
                        break;
                    case Commands::ERR:
                        cerr << "Неизвестная команда.\n";
                        break;
                }

                send(newSocket, buffer, valread, 0); // отправка преобразованного сообщения обратно клиенту
                memset(buffer, 0, sizeof(buffer)); // очистка буфера, заполнение его 0
            }
            close(newSocket);
        });
        t.detach(); // отсоединяет поток, чтобы он работал независимо от основного потока
    }
    close(serverSocket);
    return 0;
}