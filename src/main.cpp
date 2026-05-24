#include <iostream>
#include "Server.hpp"

int main()
{
    Server *s = &Server::instance();
    
    s->run(6666);

    return 0;
}