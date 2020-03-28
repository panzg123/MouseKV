//
// Created by zgpan on 2018/7/8.
//

#ifndef MOUSEKV_LEVELDB_H
#define MOUSEKV_LEVELDB_H

#include <vector>
#include <string>
#include <leveldb/db.h>
#include <leveldb/options.h>
#include "log.h"
#include "binlog.h"
using namespace std;

class LevelDb
{
public:
    LevelDb(void);
    ~LevelDb(void);
    bool setValue(const string& key,const string& value);
    bool getValue(const string& key,string& value);
    bool delKey(const string& key);
    void InitDb(const string& db_name);
    bool openDb();

private:
    leveldb::DB * m_db;
    string m_db_name;
    leveldb::Options m_options;
};

class LevelDbCluster
{

    LevelDbCluster(void);
    ~LevelDbCluster(void);
public:
    LevelDb* getLevelDbByKey(const string& key);
    bool setValue(const string& key,const string& value);
    bool getValue(const string& key,string& value);
    bool delKey(const string& key);
    bool InitCluster(int db_num,string db_dir);
    void adjustCurrentBinlogFile();
    string buildRandomBinlogFileBaseName() const;
    static LevelDbCluster* instance(void);

    //日志文件路径
    std::string getBinlogFullPath(const std::string &file_name) const
    {
        return ("./binlog/" + file_name);
    }
    BinlogFileList* binlogFileList(void) { return &m_binlogFileList; }

private:
    vector<LevelDb*> m_vec_dbs;
    int m_level_db_size = 8;  //默认8个levelDb
    string m_work_dir;
    BinLog m_curBinlog;              //当前BinLog日志文件
    BinlogFileList m_binlogFileList; //BinLog文件列表
};



#endif //MOUSEKV_LEVELDB_H
