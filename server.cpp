#include <iostream>
#include <sys/socket.h> // функции для работы с сокетами
#include <netinet/in.h> // структуры данных для портов
#include <unistd.h> // функции для работы с системными вызовами
#include <string.h>
#include <thread>

using namespace std;

int main() {
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

        thread t([newSocket] () { // новый поток для соединения
            char buffer[1024] = {0}; // буфер 1024 байта, инициализированный 0
            while (true) {
                int valread = read(newSocket, buffer, 1024); // чтение данных в буфер, valread - количество байт
                if (valread <= 0) {
                    cerr << "Клиент отсоединился\n";
                    break;
                }
                cout << "Сообщение получено: " << buffer; // вывод сообщения клиента
                for (int i = 0; i < valread; i++) { // проходимся по байтам сообщения
                    buffer[i] = toupper(buffer[i]);
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