#include "eventloop.h"
#include <cstdio>

Event::Event(void)
{
}

Event::~Event(void)
{
}

void Event::set(EventLoop *loop, evutil_socket_t sock, short flags, event_callback_fn fn, void *arg)
{
    event_assign(&m_event, loop->m_event_loop, sock, flags, fn, arg);
}

void Event::setTimer(EventLoop *loop, event_callback_fn fn, void *arg)
{
    evtimer_assign(&m_event, loop->m_event_loop, fn, arg);
}

void Event::active(int timeout)
{
    if (timeout != -1) {
        timeval val;
        val.tv_sec = (timeout / 1000);
        val.tv_usec = (timeout - val.tv_sec * 1000) * 1000;
        event_add(&m_event, &val);
    } else {
        event_add(&m_event, NULL);
    }
}

void Event::remove(void)
{
    event_del(&m_event);
}



EventLoop::EventLoop(void)
{
    m_event_loop = event_base_new();
}

EventLoop::~EventLoop(void)
{
    if (m_event_loop) {
        event_base_free(m_event_loop);
    }
}

void EventLoop::exec(void)
{
    if (m_event_loop) {
        if (event_base_dispatch(m_event_loop) < 0) {
            printf("EventLoop::exec: event_base_dispatch() failed");
        }
    }
}

void EventLoop::exit(int timeout)
{
    if (m_event_loop) {
        timeval* p = NULL;
        timeval val;
        if (timeout != -1) {
            val.tv_sec = (timeout / 1000);
            val.tv_usec = (timeout - val.tv_sec * 1000) * 1000;
            p = &val;
        }
        if (event_base_loopexit(m_event_loop, p) < 0) {
            printf("EventLoop::exit: event_base_loopexit() failed");
        }
    }
}
