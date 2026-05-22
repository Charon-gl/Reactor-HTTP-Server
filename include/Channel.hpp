#pragma once

#include <iostream>
#include <functional>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "EventLoop.hpp"


class Channel
{
private:
    int fd;
    uint32_t events;
    EventLoop *eventloop;
    
    std::function<int()> read_callback;
    std::function<int()> write_callback;

    std::function<void(int)> disconnect_callback;

    
    void update();
    bool writing_enabled;
    
    public:
    Channel(int, EventLoop *);
    
    Channel(Channel &&);
    Channel &operator=(Channel &&);
    
    void set_read_callback(std::function<int()>);
    void set_write_callback(std::function<int()>);
    
    void set_disconnect_callback(std::function<void(int)>);
    
    void event_handle(uint32_t);
    
    void do_close(int);

    bool enable_reading();
    bool enable_writing();
    bool disbale_writing();
    bool is_writing_enabled() const;
    void set_writing_enabled(bool);
    bool enable_error();
    bool clear_events();

    int get_fd() const;
    u_int32_t get_events() const;

    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    
    ~Channel();
};