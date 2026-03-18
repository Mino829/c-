#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <fstream> // ファイル読み書き用
#include <ctime>   // ログの時刻取得用

class ChatServer {
private:
    int server_fd;
    int port;
    std::vector<int> clients;
    std::mutex clients_mtx;
    std::mutex log_mtx; // ログファイルへの同時書き込みを防ぐ専用の鍵

    // 【新規】ログファイルに時刻付きで書き込むメソッド
    void log_message(const std::string& message) {
        std::lock_guard<std::mutex> lock(log_mtx);
        
        // std::ios::app をつけると「上書き」ではなく「末尾に追記」になる
        std::ofstream log_file("chat_log.txt", std::ios::app);
        if (log_file.is_open()) {
            // 現在時刻の取得
            time_t now = time(0);
            char* dt = ctime(&now);
            dt[strlen(dt) - 1] = '\0'; // ctimeが勝手につける改行を消す

            // ファイルに書き込む
            log_file << "[" << dt << "] " << message << "\n";
            log_file.close();
        }
    }

    // 全員に配るメソッド
    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(clients_mtx);
        for (int sock : clients) {
            send(sock, message.c_str(), message.length(), 0);
        }
    }

    void handle_client(int client_socket) {
        char buffer[1024];
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            
            // 切断時の処理
            if (bytes_received <= 0) {
                std::string quit_msg = "クライアントが退出しました（ID: " + std::to_string(client_socket) + "）";
                std::cout << quit_msg << "\n";
                log_message(quit_msg); // 退室記録もログに残す
                
                {
                    std::lock_guard<std::mutex> lock(clients_mtx);
                    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
                }
                close(client_socket);
                break;
            }

            std::string msg(buffer);
            std::cout << "受信: " << msg << std::endl;
            
            // 受信した生のメッセージをログに記録
            log_message(msg);

            // --- 【新規】スラッシュコマンドの解析 ---
            // メッセージは "[Mino] こんにちは" のような形なので、"] " の後ろを取り出す
            size_t pos = msg.find("] ");
            if (pos != std::string::npos) {
                std::string content = msg.substr(pos + 2); // "] "の直後から末尾までを取得

                // もし中身が "/list" だったら
                if (content == "/list") {
                    int count = 0;
                    {
                        std::lock_guard<std::mutex> lock(clients_mtx);
                        count = clients.size();
                    }
                    
                    // 【ユニキャスト】ブロードキャストはせず、発言した本人だけにこっそり送る
                    std::string reply = "[サーバー通知] 現在の接続人数は " + std::to_string(count) + " 人です。\n";
                    send(client_socket, reply.c_str(), reply.length(), 0);
                    
                    continue; // ここで処理をスキップし、他の人には見せない
                }
            }

            // コマンドでなければ、通常通り全員に配る
            broadcast(msg);
        }
    }

public:
    ChatServer(int port_num) : port(port_num), server_fd(-1) {}

    ~ChatServer() {
        if (server_fd != -1) {
            close(server_fd);
        }
    }

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
        std::cout << "高機能チャットサーバーがポート" << port << "で待機中...\n";
        
        // サーバー起動ログ
        log_message("=== サーバーを起動しました (ポート " + std::to_string(port) + ") ===");

        sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);

        while (true) {
            int client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
            if (client_socket < 0) continue;

            std::string join_msg = "新規参加者が入室しました！（ID: " + std::to_string(client_socket) + "）";
            std::cout << "\n--- " << join_msg << " ---" << std::endl;
            log_message(join_msg); // 入室記録をログに残す

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