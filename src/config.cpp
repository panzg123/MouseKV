//
// Created by zgpan on 2018/7/15.
//

#include "config.h"
#include "tinyxml2.h"
#include <cassert>

using namespace tinyxml2;

bool Config::InitConfig(string file_path)
{
    //tinyxml读取配置
    if(file_path.empty())
    {
        fprintf(stderr, "InitConfig, file_path empty\n");
        return false;
    }
    XMLDocument doc;
    int ret = doc.LoadFile(file_path.c_str());
    assert(ret==0);

    XMLElement* root = doc.FirstChildElement("config");
    if (root == nullptr)
    {
        fprintf(stderr, "InitConfig, Not find the root node[config]\n");
        return false;
    }

    //svr
    XMLElement* svr =  root->FirstChildElement("svr");
    if(svr == nullptr)
    {
        fprintf(stderr,"InitConfig, find svr element error\n");
        return false;
    }
    m_leveldb_size = svr->IntAttribute("db_num");
    m_thread_num = svr->IntAttribute("thread_num");
    m_leveldb_dir = svr->Attribute("db_dir");
    m_svr_port = svr->IntAttribute("port");
    m_log_level = svr->IntAttribute("log_level");

    //master-slave sync conf
    XMLElement* master = root->FirstChildElement("master");
    if( master != nullptr)
    {
        m_master_ip = master->Attribute("ip");
        m_master_port = master->IntAttribute("port");
        printf("master info, ip[%s] port[%d]\n", m_master_ip.c_str(), m_master_port);
    }

    //打印下配置
    fprintf(stderr,"m_leveldb_size[%d], m_thread_num[%d], m_leveldb_dir[%s], m_svr_port[%d], m_log_level[%d]\n",
                m_leveldb_size, m_thread_num, m_leveldb_dir.c_str(), m_svr_port, m_log_level);
    return true;

}


Config* Config::instance()
{
    static Config instance;
    return &instance;
}