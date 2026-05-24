#pragma once

#include <iostream>
#include <functional>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

class Channel
{
private:
    int fd;
    uint32_t events;
    
    std::function<int()> read_callback;
    std::function<int()> write_callback;
    std::function<void(Channel*)> update_events;
    std::function<void(int)> disconnect_all_callback;

    bool writing_enabled;
    
public:
    Channel(int);
    
    void set_read_callback(std::function<int()>);
    void set_write_callback(std::function<int()>);
    
    void set_disconnect_callback(std::function<void(int)>);
    void set_update_events(std::function<void(Channel *)>);
    
    int event_handle(uint32_t);
    std::function<void(int)> disconnect_callback;
    
    bool enable_reading();
    bool enable_writing();
    bool disbale_writing();
    bool is_writing_enabled() const;
    void set_writing_enabled(bool);
    bool enable_error();
    bool clear_events();
    
    int get_fd() const;
    u_int32_t get_events() const;
    
    Channel(Channel &&) = delete;
    Channel &operator=(Channel &&) = delete;
    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    
    ~Channel();
};