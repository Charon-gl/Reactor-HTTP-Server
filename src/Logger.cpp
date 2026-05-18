#include "Logger.hpp"

void Logger::add_log(char* err_str, int _errno, int _fd = -1, int err_type = 0)
{
    if(err_type == 0)
        std::cout << _fd << err_str << std::endl;
    else
        std::cout << err_type << ": " << err_str << std::endl;
}