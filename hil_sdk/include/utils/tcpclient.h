#pragma once
#include <string>
#include <functional>

class Socket;
class Packet;
class ClientSocket;

class TcpClient
{
public:
    TcpClient(std::string host, int port, int channel, std::function<bool(char*, int, std::uint16_t)> callback);
    ~TcpClient();

    bool Init();
    bool Close();
    bool Send(const char* buf, const int& len, const int& ch);
    bool SendFile(std::string filename, int ch);
private:
    bool receive_loop();
    bool receive();
private:
    bool started;
    std::function<bool(char*, int, std::uint16_t)> callback;
    std::shared_ptr<Socket> sock;
    std::shared_ptr<ClientSocket> clientSocket;
};


