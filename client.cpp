#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <thread>

class ChatClient {
private:
    int sock;
    std::string server_ip;
    int port;
    std::string username; // クライアント自身の名前を記憶する変数

    // サーバーからのメッセージを常に待ち受けるメソッド（別スレッド用）
    void receive_messages() {
        char buffer[1024];
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
            
            if (bytes_received <= 0) {
                std::cout << "\nサーバーから切断されました。\n";
                break;
            }

            // メッセージを表示した後、自分の入力プロンプトを再度表示する
            std::cout << "\n" << buffer << "\n" << username << ": " << std::flush;
        }
    }

public:
    // コンストラクタ：IPアドレスとポート番号を受け取る
    ChatClient(std::string ip, int port_num) : server_ip(ip), port(port_num), sock(-1) {}

    // デストラクタ：終了時にソケットを閉じる
    ~ChatClient() {
        if (sock != -1) {
            close(sock);
        }
    }

    // クライアントの起動とメインループ
    void start() {
        // 1. まず最初にユーザー名を入力させる
        std::cout << "チャットで使う名前を入力してください: ";
        std::getline(std::cin, username);

        // 2. ソケットの作成と接続
        sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "接続失敗。サーバーは起動していますか？\n";
            return;
        }

        std::cout << "サーバーに接続しました！（'quit' と入力で終了）\n";

        // 3. 受信専用スレッドを起動
        std::thread(&ChatClient::receive_messages, this).detach();

        // 4. メインループ（送信処理）
        std::string input;
        while (true) {
            std::cout << username << ": ";
            std::getline(std::cin, input);

            if (input == "quit") {
                break;
            }

            // 【ポイント】送信するテキストの先頭に [ユーザー名] を付与する
            std::string message = "[" + username + "] " + input;
            send(sock, message.c_str(), message.length(), 0);
        }
    }
};

int main() {
    ChatClient client("127.0.0.1", 8080);
    client.start();
    
    return 0;
}