## C++ Multi-threaded Chat System
C++とPOSIXソケットを用いた、シンプルな複数人同時参加型のチャットシステムです。サーバーが複数のクライアント接続をスレッドで管理し、送られたメッセージを参加者全員にブロードキャストします。
1. 機能概要マルチクライアント対応: std::thread を使用し、複数の接続を並行して処理します。リアルタイム・ブロードキャスト: 一人の発言を、接続中の全ユーザーへ即時に転送します。スレッドセーフな管理: std::mutex を用いて、クライアントリストへのアクセスを保護しています。POSIX準拠: Windows (Cygwin/MSYS2) だけでなく、LinuxやmacOSでも動作する移植性の高いコードです。
2. 開発環境OS: Windows 11 (Cygwin環境)Compiler: g++ (GCC) 15.x 以上Language: C++11 以降Libraries: Standard Library (iostream, thread, vector, mutex), POSIX Socket API
3. ビルド方法ターミナルで以下のコマンドを実行してコンパイルします。Bash# サーバーのビルド（スレッドライブラリをリンク）
g++ server.cpp -o server.exe -pthread

# クライアントのビルド
g++ client.cpp -o client.exe
4. 使用方法サーバーの起動:Bash./server.exe
クライアントの接続:別のターミナルを複数開き、それぞれで実行します。Bash./client.exe
チャットの終了:クライアント側で quit と入力すると、安全に切断されます。5. 技術的なポイント使用技術用途TCP Sockets信頼性の高い双方向通信の実現std::thread接続ごとの独立したメッセージ受信処理std::mutexクライアントリスト（std::vector）の排他制御std::remove切断されたソケットの効率的なリスト削除