#pragma once
#include <cstdint>
#include <string>
#include <memory>

class Socket;

class Packet
{
public:
    Packet();
    ~Packet();

    bool Encode(const char* buf, const int& len, const int& ch, const uint8_t& code);
    bool Decode(std::shared_ptr<Socket> sock);

    bool IsRegisterCode();
    bool IsSendFileCode();
    
    static std::shared_ptr<Packet> EncodeFromFile(std::string filename, int ch);
private:
    int packet_len(const int& data_len);
    bool decode_header(std::shared_ptr<Socket> sock);
    bool decode_body(std::shared_ptr<Socket> sock);

private:
    std::uint8_t version;
    std::uint8_t code;   // 1:register 2:send file
    std::uint8_t magic;
public:
    std::uint16_t channel;
    std::uint32_t buffer_len;
    char *buffer;
};
