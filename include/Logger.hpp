#include <iostream>

class Logger
{
private:
    Logger() = default;

public:
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;

    static void add_log(const char*, int _errno, int _fd = -1, int err_type = 0);
};