#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h>    
#include <cstring> 

int main() {
    // 1. ソケット作成 
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "ソケットの作成に失敗しました\n";
        return 1;
    }

    // 2. アドレスとポートの設定
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // 3. バインド (ポートの確保)
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "バインドに失敗しました\n";
        return 1;
    }

    // 4. 待機開始
    if (listen(server_fd, 3) < 0) {
        std::cerr << "待機状態への移行に失敗しました\n";
        return 1;
    }
    
    std::cout << "サーバーがポート8080で待機中..." << std::endl;

    sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);

    while (true){
        //5. accept: クライアントからの接続を待つ
        int client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
        if(client_socket < 0){
            std::cerr << "接続の受付に失敗しました\n";
            continue;
        }

        //6. メッセージの受信
        char buffer[1024] = {0}; //受信用のバッファを用意
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::cout << "受信したメッセージ:\n" << buffer << std::endl;

        std::cout << "\n--- クライアントが接続しました！ ---" << std::endl;
        //7. メッセージの返信
        const char* reply = "Hello from C++ Server!\n";
        send(client_socket, reply, strlen(reply), 0);
        std::cout << "返信を送信しました。" << std::endl;


    }
    
    close(server_fd);
    
    return 0;
}