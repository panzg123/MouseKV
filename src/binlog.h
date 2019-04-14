//
// Created by zgpan on 2019/4/14.
//

#ifndef MOUSEKV_BINLOG_H
#define MOUSEKV_BINLOG_H

#include <string>
#include <cstdio>

using namespace std;

class BinLog
{
    struct Header
    {
        int handshake = 0x1ecdf00; //这是一个魔法数
    };

    struct LogItem
    {
        //binlog操作类型--两种
        enum {
            SET,
            DEL
        };
        int item_size;  //这条LogItem大小
        int type;       //操作类型
        int key_size;   //key长度
        int value_size; //value长度

        char* keyBuffer(void) { return (char*)(this+1); }
        char* valueBuffer(void) { return keyBuffer()+key_size; }
    };

    BinLog();
    ~BinLog();
    bool open(const std::string& file_name);
    std::string fileName() const { return m_fileName; }
    size_t writtenSize() const { return m_writtenSize; }
    void sync();
    void close();
    bool appendSetRecord(const string& key, const string& value);
    bool appendDelRecord(const string& key);

private:
    std::string m_fileName;   //当前binlog文件名
    FILE* m_fp;               //文件读写file*
    size_t m_writtenSize;     //已经写入得长度
    BinLog(const BinLog&);
    BinLog& operator=(const BinLog&);
};




#endif //MOUSEKV_BINLOG_H
