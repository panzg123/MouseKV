//
// Created by zgpan on 2019/4/14.
//

#include "binlog.h"
#include "log.h"

BinLog::BinLog()
{
    m_fileName = "";
    m_fp = nullptr;
    m_writtenSize = 0;
}

BinLog::~BinLog()
{
    close();
}

bool BinLog::open(const std::string &file_name)
{
    close();

    //打开文件
    FILE* fp = fopen(file_name.c_str(),"a+");
    if(!fp)
    {
        COMM_LOG(Logger::ERROR,"open binlog failed");
        return false;
    }

    //将文件指针指向末尾
    if(fseek(fp,0,SEEK_END) != 0)
    {
        fclose(fp);
        COMM_LOG(Logger::ERROR,"seek binlog failed");
        return false;
    }

    //如果没有头，则写入头

}