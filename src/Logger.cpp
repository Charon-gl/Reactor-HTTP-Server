#include "Logger.hpp"

void Logger::add_log(int _fd = -1, char* err_str, int err_type = 0)
{
    if(err_type == 0)
        std::cout << "连接" << _fd << ": " << err_str << std::endl;
    else
        std::cout << "全局异常" << ": " << err_str << std::endl;
}