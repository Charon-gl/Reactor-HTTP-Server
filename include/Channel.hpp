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
    
    std::function<void()> read_callback;
    std::function<void()> write_callback;
    std::function<void()> error_callback;
    void update();
    bool writing_enabled;

public:
    Channel(int, EventLoop*);
    
    Channel(Channel &&);
    Channel &operator=(Channel &&);

    void set_read_callback(std::function<void()>);
    void set_write_callback(std::function<void()>);
    void set_error_callback(std::function<void()>);

    void event_handle(uint32_t);

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