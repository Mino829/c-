#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

// サーバーの機能をまとめたクラス
class ChatServer {
private:
    int server_fd;
    int port;
    std::vector<int> clients;
    std::mutex clients_mtx;

    // 特定のメッセージを全員に送信する
    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(clients_mtx); // スコープを抜けると自動でunlockされるC++の便利機能
        for (int sock : clients) {
            send(sock, message.c_str(), message.length(), 0);
        }
    }

    // 個別のクライアントの相手をする処理
    void handle_client(int client_socket) {
        char buffer[1024];
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            
            if (bytes_received <= 0) {
                std::cout << "クライアントが退出しました（ID: " << client_socket << "）\n";
                
                // リストから削除
                {
                    std::lock_guard<std::mutex> lock(clients_mtx);
                    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
                }
                close(client_socket);
                break;
            }

            std::cout << "受信: " << buffer << std::endl;
            std::string message = "User" + std::to_string(client_socket) + ": " + std::string(buffer);
            
            // 独立したメソッドを呼び出すだけ
            broadcast(message);
        }
    }

public:
    ChatServer(int port_num) : port(port_num), server_fd(-1) {}

    ~ChatServer() {
        if (server_fd != -1) {
            close(server_fd);
        }
    }

    // サーバーの起動とメインループ（外部から呼び出せるのはこれだけ）
    void start() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "バインド失敗\n";
            return;
        }

        listen(server_fd, 3);
        std::cout << "チャットサーバーがポート" << port << "で待機中...\n";

        sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);

        while (true) {
            int client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
            if (client_socket < 0) continue;

            std::cout << "\n--- 新規参加者が入室しました！（ID: " << client_socket << "） ---" << std::endl;

            {
                std::lock_guard<std::mutex> lock(clients_mtx);
                clients.push_back(client_socket);
            }

            
            std::thread(&ChatServer::handle_client, this, client_socket).detach();
        }
    }
};


int main() {
    ChatServer server(8080);
    server.start();
    
    return 0;
}