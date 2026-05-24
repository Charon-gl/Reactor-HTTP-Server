#include "Logger.hpp"

void Logger::add_log(const char* err_str, int _fd, int err_type)
{
    if(err_type == 0)
        std::cout << "连接" << _fd << ": " << err_str << std::endl;
    else
        std::cout << "全局异常" << ": " << err_str << std::endl;
}