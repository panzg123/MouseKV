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
    string m_leveldb_dir = "./data/";
    int m_thread_num = 8;
    int m_svr_port = 10086;
    int m_log_level = 0;

};



#endif //MOUSEKV_CONFIG_H
