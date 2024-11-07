#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

using namespace std;

int main() {
    cout << "Launching server...\n";
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "Could not create socket\n";
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8081);

    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cerr << "Bind failed\n";
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed\n";
        close(server_fd);
        return 1;
    }

    cout << "Waiting for incoming connections...\n";

    sockaddr_in client_address;
    socklen_t client_address_lenght = sizeof(client_address);
    int new_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_address_lenght);
    if (new_socket < 0) {
        cerr << "Accept failed\n";
        close(server_fd);
        return 1;
    }

    cout << "Connection accepted\n";

    char buffer[1024] = {0};
    while (true) {
        int valread = read(new_socket, buffer, 1024);
        if (valread <= 0) {
            cerr << "Client disconnected\n";
            break;
        }

        cout << "Message received: " << buffer;

        for (int i = 0; i < valread; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        send(new_socket, buffer, valread, 0);
        memset(buffer, 0, sizeof(buffer));
    }

    close(new_socket);
    close(server_fd);
    return 0;
}