//
// Created by zgpan on 2020/3/29.
//

#include "DbCopyThread.h"


int DbCopyThread::start()
{
    m_is_running = true;
    return pthread_create(&m_thread_id,nullptr,run, this);
}

void* DbCopyThread::run(void *arg)
{
    DbCopyThread* thread = (DbCopyThread*)arg;

    //TODO 循环遍历leveldb，发送给dst svr
    COMM_LOG(Logger::DEBUG,"copy thread running...");

}

void DbCopyThread::terminate()
{
    m_is_running = false;
}



