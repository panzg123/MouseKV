# MouseKV
oh, something cool.

base from [OneValue](https://github.com/onexsoft/OneValue)

项目简介：
1. 多线程Reactor模型，主线程监听连接请求，8子线程处理收发包和业务逻辑，底层配置多个LevelDb构成LevelDbCluster
2. 支持主从同步，Binlog实现机制


## 图解

##### 项目ULM

#### BinLog机制

关键类：
- LogItem 一条日志信息
- BinLog 单个日志文件
- BinlogFileList 日志文件列表类
- BinlogSyncStream 主从同步时的一条数据流
- BinlogBufferReader 主节点读取binLog记录类

单条操作binLog日志信息，LogItem示意图如下

![Image text](https://github.com/panzg123/MouseKV/blob/master/images/MouseKv_LogItem.png)


## TODO
- [x] Log
- [x] levelDb
- [x] binlog
- [x] master-slave
- [ ] restful api
- [x] multi work process
- [ ] support cas
- [ ] support set,list,hash


## ChangeLog
20180714 Tag1.0 单线程done，默认配置8个leveldb。


## BenchMark

    ubuntu@VM-0-3-ubuntu:~/tool/redis-4.0.0/src$ ./redis-benchmark -h localhost -p 10086 -t set,get -c 10 -n 10000 -q
    SET: 14306.15 requests per second
    GET: 18315.02 requests per second

作为对比，redis4.0 benchmark信息

    ubuntu@VM-0-8-ubuntu:~/github/redis-4.0.10/src$ ./redis-benchmark -h localhost -t set,get -c 10 -n 10000 -q
    SET: 52356.02 requests per second
    GET: 51282.05 requests per second

### Environment

	腾讯云 1核 1GB 2Mbps
