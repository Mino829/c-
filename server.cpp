#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    std::cout << "サーバーがポート8080で待機中...\n";

    sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);

    while (true) {
        int client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
        if (client_socket < 0) continue;

        std::cout << "\n--- クライアントが接続しました！ ---" << std::endl;

        char buffer[1024];
        while (true) {
            memset(buffer, 0, sizeof(buffer)); 
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            
            
            if (bytes_received <= 0) {
                std::cout << "クライアントが退出しました。\n";
                break; // 内側のループを抜ける
            }

            std::cout << "受信: " << buffer << std::endl;

            
            std::string reply = "Server Echo: " + std::string(buffer);
            send(client_socket, reply.c_str(), reply.length(), 0);
        }

        close(client_socket); 
    }
    
    close(server_fd);
    return 0;
}