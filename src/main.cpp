#include <iostream>
#include <signal.h>
#include "Server.hpp"

int main()
{
    signal(SIGPIPE, SIG_IGN); // 忽略 SIGPIPE
    Server *s = &Server::instance();
    
    s->run(6666);

    return 0;
}