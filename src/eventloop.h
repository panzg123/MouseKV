#ifndef EVENTLOOPPOOL_H
#define EVENTLOOPPOOL_H

#include <pthread.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <vector>

using namespace std;

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


class EventLoopThread
{
public:
    EventLoopThread(void): m_is_running(false) {}
    ~ EventLoopThread(void)
    {
        if(m_is_running)
            terminate();
    }
    void start(void);
    static void* run(void* arg);
    void terminate(void);

public:
    Event m_timeout;
    EventLoop m_event_loop;
    pthread_t m_thread_id;
    bool m_is_running;
};


class EventLoopThreadPool
{

public:
    EventLoopThreadPool(void){}
    ~EventLoopThreadPool(void);
    bool initThreadPool();
    EventLoop* getEventLoop();
    static EventLoopThreadPool* instance();
private:
    int m_default_thread_num = 8; //TODO 可配置，目前默认8个线程，8个levelDb
    vector<EventLoopThread*> m_threads;
    unsigned int m_cur_thread_ref; //当前线程引用计数
};
#endif