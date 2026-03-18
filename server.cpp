#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm> 


std::vector<int> clients;
std::mutex clients_mtx; 

// 個々のクライアントの相手をする専用関数（スレッドとして動く）
void handle_client(int client_socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        
        // 切断された場合
        if (bytes_received <= 0) {
            std::cout << "クライアントが退出しました（ID: " << client_socket << "）\n";
            
            // リストからこのクライアントを削除する
            clients_mtx.lock();
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
            clients_mtx.unlock();
            
            close(client_socket);
            break; // このスレッドのお仕事終了
        }

        std::cout << "受信: " << buffer << std::endl;

        // 【ブロードキャスト】
        std::string message = "User" + std::to_string(client_socket) + ": " + std::string(buffer);
        
        clients_mtx.lock();
        for (int sock : clients) {
            // 自分を含む、リストにいる全員に送信
            send(sock, message.c_str(), message.length(), 0);
        }
        clients_mtx.unlock();
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    std::cout << "チャットサーバーがポート8080で待機中...\n";

    sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);

    while (true) {
        
        int client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
        if (client_socket < 0) continue;

        std::cout << "\n--- 新規参加者が入室しました！（ID: " << client_socket << "） ---" << std::endl;

        
        clients_mtx.lock();
        clients.push_back(client_socket);
        clients_mtx.unlock();

        
        std::thread(handle_client, client_socket).detach();
    }
    
    close(server_fd);
    return 0;
}