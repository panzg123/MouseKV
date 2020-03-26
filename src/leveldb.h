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
    static LevelDbCluster* instance(void);

private:
    vector<LevelDb*> m_vec_dbs;
    int m_level_db_size = 8;  //默认8个levelDb
    string m_work_dir;
    BinLog m_curBinlog;
};



#endif //MOUSEKV_LEVELDB_H
