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
        event_add(&m_event, nullptr);
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
            fprintf(stderr,"EventLoop::exec: event_base_dispatch() failed");
        }
    }
}

void EventLoop::exit(int timeout)
{
    if (m_event_loop) {
        timeval* p = nullptr;
        timeval val;
        if (timeout != -1) {
            val.tv_sec = (timeout / 1000);
            val.tv_usec = (timeout - val.tv_sec * 1000) * 1000;
            p = &val;
        }
        if (event_base_loopexit(m_event_loop, p) < 0) {
            fprintf(stderr,"EventLoop::exit: event_base_loopexit() failed");
        }
    }
}


//=========================================================线程========================================================
void EventLoopThread::start(void)
{
    pthread_create(&m_thread_id,nullptr,run, this);
}

void emptyCallBack(evutil_socket_t, short, void*)
{
}

void EventLoopThread::terminate(void)
{
    m_timeout.remove();
    m_event_loop.exit();
}

void* EventLoopThread::run(void* arg)
{
    EventLoopThread* thread = (EventLoopThread*)arg;
    thread->m_timeout.set(&thread->m_event_loop, -1, EV_PERSIST | EV_TIMEOUT, emptyCallBack, thread);
    thread->m_timeout.active(60000);
    thread->m_event_loop.exec();
    thread->m_is_running = true;
    return nullptr;
}

//===========================================================线程池=====================================================

bool EventLoopThreadPool::initThreadPool(int thread_num)
{
    fprintf(stderr,"EventLoopThreadPool::initThreadPool begin\n");
    m_default_thread_num = thread_num;
    for (int i = 0; i < m_default_thread_num; ++i)
    {
        EventLoopThread *thread = new EventLoopThread;
        thread->start();
        m_threads.push_back(thread);
    }
    return true;
}


EventLoop* EventLoopThreadPool::getEventLoop()
{
    int index = m_cur_thread_ref%m_default_thread_num;
    ++m_cur_thread_ref;
    return &m_threads[index]->m_event_loop;
}

EventLoopThreadPool* EventLoopThreadPool::instance()
{
    static EventLoopThreadPool pool;
    return &pool;
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    for (int i = 0; i < m_default_thread_num; ++i)
    {
        m_threads[i]->terminate();
        delete m_threads[i];
    }
}