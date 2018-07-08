//
// Created by zgpan on 2018/7/8.
//

#ifndef MOUSEKV_LEVELDB_H
#define MOUSEKV_LEVELDB_H

#include <vector>
#include <string>
#include <leveldb/db.h>
#include <leveldb/options.h>
using namespace std;

class LevelDb
{
public:
    LevelDb(void);
    ~LevelDb(void);
    bool setValue(const string& key,const string& value);
    bool getValue(const string& key,string& value);
    void InitDb(const string& db_name);
    bool openDb();

private:
    leveldb::DB * m_db;
    string m_db_name;
    leveldb::Options m_options;
};

class LevelDbCluster
{
public:
    LevelDbCluster(void);
    ~LevelDbCluster(void);
    LevelDb* getLevelDbByKey(const string& key);
    bool setValue(const string& key,const string& value);
    bool getValue(const string& key,string& value);
    bool InitCluster();
    static LevelDbCluster* instance(void);

private:
    vector<LevelDb*> m_vec_dbs;
    const int m_level_db_size = 8;
    string m_work_dir;
};



#endif //MOUSEKV_LEVELDB_H
