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
    inet_pton(AF_INET, "192.168.88.130", &addr.sin_addr.s_addr);
    addr.sin_port = htons(host_port);

    int ret = connect(fd, (sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        throw std::runtime_error("Connect failed");
    }

    while (1)
    {
        char buf[1024];
        snprintf(buf, sizeof(buf), "Hello Server");
        ret = send(fd, buf, strlen(buf), 0);
        if (ret == -1)
        {
            std::cerr << "Send failed" << std::endl;
            continue;
        }
        std::cout << "I say: " << buf << std::endl;
        ret = recv(fd, buf, sizeof(buf) - 1, 0);
        if (ret == 0)
        {
            std::cout << "Server disconnected..." << std::endl;
            return;
        }
        else if (ret == -1)
        {
            std::cerr << "Recive message failed" << std::endl;
        }
        else
        {
            buf[ret] = '\0';
            std::cout << "Server say: " << buf << std::endl;
        }
    }
}

Client::~Client()
{
    if (fd != -1)
        close(fd);
}