#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string> 

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "接続失敗\n";
        return 1;
    }

    std::cout << "サーバーに接続しました！（'quit' と入力で終了）\n";

    char buffer[1024];
    std::string input;

    
    while (true) {
        std::cout << "あなた: ";
        std::getline(std::cin, input); 

        if (input == "quit") {
            break; 
        }

        // サーバーに送信
        send(sock, input.c_str(), input.length(), 0);

        // サーバーからの返信を待つ
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        
        if (bytes_received <= 0) {
            std::cout << "サーバーが切断されました。\n";
            break;
        }

        std::cout << buffer << std::endl;
    }

    close(sock);
    return 0;
}