//
// Created by zgpan on 2019/6/23.
//

#ifndef MOUSEKV_SYNC_H
#define MOUSEKV_SYNC_H

#include <pthread.h>
#include <vector>
#include <string>
#include "socket.h"
#include "leveldb.h"
#include "mouse.h"

using namespace std;

class SyncThread
{
public:
    SyncThread(const char* master, int port, Mouse* svr);
    ~ SyncThread();
    void start(void);
    static void* run(void* arg);
    void terminate(void);

public:
    pthread_t m_thread_id;
    bool m_is_running = false;
    HostAddress m_master_addr;  //主节点
    string m_binlog_index_file = "MASTER_INFO"; //master binlog 同步的索引文件
    std::vector<std::string> m_master_sync_info;  //不断更新的同步信息
    Socket m_master_sock;       //与主机建立的同步连接
    char   *m_net_buf;          //收发包的buf
    Mouse* server;              //mouse kv server
};



#endif //MOUSEKV_SYNC_H
