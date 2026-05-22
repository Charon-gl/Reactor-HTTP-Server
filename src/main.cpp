#include <iostream>
#include "Server.hpp"

int main()
{
    Server *s = &Server::instance();
    s->run(8888);

    return 0;
}