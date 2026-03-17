#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstring>

int main(){
    //1.ソケット作成
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        std::cerr << "ソケット作成失敗\n";
        return 1;
    }

    //2.接続先のアドレス設定
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
        std::cerr << "無効なアドレスです。\n";
        return 1;
    }

    //3.サーバーに接続
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cerr << "接続失敗。サーバーは起動していますか？\n";
        return 1;
    }

    std::cout << "サーバーに接続しました！メッセージを送信します...\n";

    //4. メッセージの送信
    const char* hello = "Hello from Client!";
    send(sock, hello, strlen(hello), 0);

    //5.　サーバーからの返信を受信
    char buffer[1024] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    std::cout << "サーバーからの返信: " << buffer << std::endl;

    //6. 通信終了
    close(sock);

    return 0;
}