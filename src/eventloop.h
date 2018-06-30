#ifndef EVENTLOOPPOOL_H
#define EVENTLOOPPOOL_H

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>


//前置声明
class EventLoop;
class Event
{
public:
    Event(void);
    ~Event(void);

    //Set event param
    void set(EventLoop* loop, evutil_socket_t sock, short flags, event_callback_fn fn, void* arg);

    //Set timeout event
    void setTimer(EventLoop* loop, event_callback_fn fn, void* arg);

    //Active
    void active(int timeout_msec = -1);

    //Remove event from event loop
    void remove(void);

private:
    event m_event;
    friend class EventLoop;
};


class EventLoop
{
public:
    EventLoop(void);
    ~EventLoop(void);

    void exec(void);
    void exit(int timeout = -1);

private:
    event_base* m_event_loop;
    friend class Event;
    EventLoop(const EventLoop&);
    EventLoop& operator=(const EventLoop&);
};

#endif