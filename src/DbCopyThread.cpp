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

    //1. 先建立同请求节点的连接--此时本节点为client
    Socket sock = Socket::CreateSocket();
    HostAddress stAddr(thread->dst_ip.c_str(), thread->dst_port);
    bool bRet = sock.connect(stAddr);
    if(!bRet)
    {
        COMM_LOG(Logger::ERROR, "connect failed, ip[%s] port[%d]", thread->dst_ip.c_str(), thread->dst_port);
        return nullptr;
    }

    //2. 遍历database
    if(thread->mouse_svr == nullptr)
    {
        COMM_LOG(Logger::ERROR, "mouse svr ptr null");
        return nullptr;
    }


    auto pDbCluster = thread->mouse_svr->getDbCluster();
    int iAllDbKeyCnt = 0;
    for (int i = 0; i < pDbCluster->databaseCount(); ++i)
    {
        LevelDb* pLevelDb = pDbCluster->database(i);
        auto pIter = pLevelDb->getIterator();
        if(pIter == nullptr)
        {
            COMM_LOG(Logger::ERROR,"get new iterator failed, db_index[%d]", i);
            continue;
        }
        pIter->SeekToFirst();
        IOBuffer sendbuff;
        int iDbKeyCnt = 0;
        while(pIter->Valid())
        {
            string key = pIter->key().ToString();
            string value = pIter->value().ToString();
            COMM_LOG(Logger::DEBUG,"find one, key[%s] value[%s]", key.c_str(), value.c_str());

            //构建buf
            sendbuff.append("*3\r\n$3\r\nSET\r\n", 13);
            sendbuff.appendFormatString("$%d\r\n", key.length());
            sendbuff.append(key.data(), key.length());
            sendbuff.append("\r\n", 2);
            sendbuff.appendFormatString("$%d\r\n", value.length());
            sendbuff.append(value.data(), value.length());
            sendbuff.append("\r\n", 2);

            ++iDbKeyCnt; //总数统计
            ++iAllDbKeyCnt;
            //直接同步到接收节点
            int sendBytes = sock.send(sendbuff.data(), sendbuff.size());
            if(sendBytes <= 0 ) //发送失败
            {
                COMM_LOG(Logger::ERROR, "send buf failed, db_index[%d]", i);
                sock.close();
                return nullptr;
            }
            sendbuff.clear();
            pIter->Next();
        }
        COMM_LOG(Logger::DEBUG, "db_index[%d] key_cnt[%d]", i, iDbKeyCnt);
    }
    sock.close();
    COMM_LOG(Logger::DEBUG, "db_size[%d] all_key_cnt[%d]", pDbCluster->databaseCount(), iAllDbKeyCnt);
    return nullptr;
}

void DbCopyThread::terminate()
{
    m_is_running = false;
}



