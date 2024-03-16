#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>

class Packet;

class Socket
{
public:
    Socket() = default;
    virtual ~Socket() = default;
    
    virtual int  Fd() = 0;
    virtual int  Channel() = 0;
    virtual void SetChannel(int ch) = 0;
    virtual bool Send(char* buf, int len) = 0;
    virtual bool Readfull(char** buf, int len) = 0;
    virtual bool Close() = 0;
};

class LinuxSocket : public Socket
{
public:
    LinuxSocket(int sockfd);
    virtual ~LinuxSocket();

    int  Fd() override;
    int  Channel() override;
    void SetChannel(int ch) override;
    bool Send(char* buf, int len) override;
    bool Readfull(char** buf, int len) override;
    bool Close() override;
private:
    int sockfd;
    bool closed;
    int channel;
};

class WinSocket : public Socket
{
public:
    WinSocket();
    virtual ~WinSocket();

    int  Fd() override;
    int  Channel() override;
    void SetChannel(int ch) override;
    bool Send(char* buf, int len) override;
    bool Readfull(char** buf, int len) override;
    bool Close() override;
};

class ServerSocket
{
public:
    ServerSocket(int port);
    ~ServerSocket();

    bool Start(std::function<bool(std::shared_ptr<Socket>)> select_callback);
    bool Send(int ch, std::shared_ptr<Packet> packet);
    bool SendAll(std::shared_ptr<Packet> packet);
private:
    bool loop_accept();
    void socket_select(std::function<bool(std::shared_ptr<Socket>)> callback);
    bool socket_listen();
    void socket_close();

    void add_client(std::shared_ptr<Socket> sock);
    void del_client(std::shared_ptr<Socket> sock);
private:
    int port;
    bool started;
    int sockfd;
    std::mutex mtx;
    std::map<int, std::shared_ptr<Socket>> sockets;
};

class ClientSocket
{
public:
    ClientSocket(std::string host, int port, int channel);
    virtual ~ClientSocket() = default;

    std::shared_ptr<Socket> Connect();
    void LoopReceive(std::shared_ptr<Socket> sock);
    void SetReceiveCallback(std::function<bool()> cb);
private:
    int port;
    int channel;
    std::string host;
    std::function<bool()> callback;
};
