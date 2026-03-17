#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h>     

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
        close(server_fd);
        return 1;
    }

    // 4. 待機開始
    if (listen(server_fd, 3) < 0) {
        std::cerr << "待機状態への移行に失敗しました\n";
        close(server_fd);
        return 1;
    }
    
    std::cout << "サーバーがポート8080で待機中..." << std::endl;

    
    close(server_fd);
    
    return 0;
}