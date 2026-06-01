#include <iostream>
#include <signal.h>
#include "Server.hpp"

int main()
{
    signal(SIGPIPE, SIG_IGN); // 忽略 SIGPIPE
    Server *s = &Server::instance("/home/charon/Reactor_Server/Log/logger");
    
    s->run(6666);

    return 0;
}