#include "Client.hpp"

Client::Client(int _host_port) : fd(-1), host_port(_host_port) {}

void Client::run()
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        throw std::runtime_error("Socket failed");
    }
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
    addr.sin_port = htons(host_port);

    int ret = connect(fd, (sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        throw std::runtime_error("Connect failed");
    }


    std::string send_buf = "GET /nothing.txt HTTP/1.0\r\n\r\n";
    
    ret = send(fd, send_buf.data(), send_buf.size(), 0);
    if (ret == -1)
    {
        std::cerr << "Send failed" << std::endl;
    }
    std::cout << "I say: " << send_buf << std::endl;
    std::string recv_buf;
    while(1)
    {
        char tmp[10];
        ret = recv(fd, tmp, sizeof(tmp), 0);
        if(ret > 0)
        {
            recv_buf += tmp;
        }
        else if (ret == 0)
        {
            std::cout << "Server disconnected..." << std::endl;
            break;
        }
        else if (ret == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                std::cerr << "recv failed" << std::endl;
                return;
            }
        }
    }
    recv_buf[recv_buf.size()] = '\0';
    std::cout << "Server say: " << recv_buf << std::endl;
}

Client::~Client()
{
    if (fd != -1)
        close(fd);
}