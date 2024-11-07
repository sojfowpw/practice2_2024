#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

using namespace std;

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Could not create socket\n";
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8081);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported\n";
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cerr << "Connection failed\n";
        close(sock);
        return 1;
    }

    char buffer[1024] = {0};
    while (true) {
        cout << "Text to send: ";
        string text;
        getline(cin, text);

        send(sock, text.c_str(), text.length(), 0);
        send(sock, "\n", 1, 0);

        int valread = read(sock, buffer, 1024);
        if (valread <= 0) {
            cerr << "Server disconnected\n";
            break;
        }

        cout << "Message from server: " << buffer;
        memset(buffer, 0, sizeof(buffer));
    }
    close(sock);
    return 0;
}