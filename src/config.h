//
// Created by zgpan on 2018/7/15.
//

#ifndef MOUSEKV_CONFIG_H
#define MOUSEKV_CONFIG_H

#include <vector>
#include <string>
using namespace std;


class Config
{
private:
    Config(){}
    ~Config(){}
public:
    bool InitConfig(string file_path);
    static Config* instance();

public:
    int m_leveldb_size = 8;
    string m_leveldb_dir = "./data/";  //level目录
    string m_binlog_dir  = "./binlog"; //日志目录
    int m_thread_num = 8;    //work线程个数
    int m_svr_port = 10086;  //svr端口
    int m_log_level = 0;     //日志级别

    string m_master_ip = ""; //从节点时使用-master ip
    int m_master_port = 0;   //从节点时使用-master port
};

#endif //MOUSEKV_CONFIG_H
