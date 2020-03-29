//
// Created by zgpan on 2020/3/29.
//

#ifndef MOUSEKV_DBCOPYTHREAD_H
#define MOUSEKV_DBCOPYTHREAD_H

#include <pthread.h>
#include <string>
#include "socket.h"
#include "leveldb.h"
#include "mouse.h"

class DbCopyThread
{
public:
    DbCopyThread(Mouse* mouse_svr, const string& dst_ip, int dst_port):
        mouse_svr(mouse_svr), dst_ip(dst_ip), dst_port(dst_port) {}
    ~ DbCopyThread()
    {
        if(m_is_running)
            terminate();
    }
    int start();
    static void* run(void* arg);
    void terminate();

public:

    pthread_t m_thread_id;
    bool m_is_running = false;
    Mouse* mouse_svr = nullptr;
    string dst_ip = "";
    int dst_port = 0;
};

#endif //MOUSEKV_DBCOPYTHREAD_H
