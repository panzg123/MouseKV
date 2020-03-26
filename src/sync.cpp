//
// Created by zgpan on 2019/6/23.
//

#include "sync.h"
#include "binlog.h"
#include "log.h"


SyncThread::SyncThread(const char *master, int port)
{
    m_master_addr  = HostAddress(master, port);
}


void SyncThread::terminate()
{
    m_is_running = false;
}

void* SyncThread::run(void *arg)
{
    SyncThread* thread = (SyncThread*)arg;
    //0. 读取索引文件记录的位置信息 -- 如果没有读取，则初始化为默认值
    bool ret = TextConfigFile::read(thread->m_binlog_index_file, thread->m_master_sync_info);
    if(ret){
        if(thread->m_master_sync_info.size() != 2){
            COMM_LOG(Logger::ERROR, "m_master_sync_info error, size[%zu]", thread->m_master_sync_info.size());
            return NULL;
        }
    } else{
        thread->m_master_sync_info.clear();
        thread->m_master_sync_info.emplace_back(" ");
        thread->m_master_sync_info.emplace_back("-1");
    }

    //1.创建tcp链接
    thread->m_master_sock = Socket::CreateSocket();
    if (!thread->m_master_sock.connect(thread->m_master_addr))
    {
        COMM_LOG(Logger::ERROR,"connect failed, master_ip[%s] master_port[%d]",
                thread->m_master_addr.ip(), thread->m_master_addr.port());
        return NULL;
    }

    //2. while循环--发送SYNC命令、接受同步数据、处理同步的数据
    while (thread->m_is_running)
    {
        //组装包
        int _send_len = sprintf(thread->m_net_buf,"*3\r\n$6\r\n__SYNC\r\n$%d\r\n%s\r\n$%d\r\n%s\r\n",
                                (int)thread->m_master_sync_info[0].length(), thread->m_master_sync_info[0].c_str(),
                                (int)thread->m_master_sync_info[1].length(), thread->m_master_sync_info[1].c_str());
        int _send_succ_len = thread->m_master_sock.send(thread->m_net_buf, _send_len);
        if (_send_succ_len < 0) {
            //发送失败了
            COMM_LOG(Logger::ERROR,"send failed, _send_len[%d] _send_succ_len[%d]", _send_len, _send_succ_len);
            continue;
        }
        COMM_LOG(Logger::DEBUG,"send succ, _send_len[%d] _send_succ_len[%d]", _send_len, _send_succ_len);

        //收包
        int _buf_len = sizeof(m_net_buf);
        int _recv_len = thread->m_master_sock.recv(thread->m_net_buf, _buf_len);


        //处理包
    }
}


void SyncThread::start()
{
    pthread_create(&m_thread_id,NULL,run, this);
}

