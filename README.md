# MouseKV
oh, something cool.

### TODO
- [x] Log
- [x] levelDb
- [ ] master-slave
- [ ] restful api
- [ ] multi work process


### ChangeLog
20180714 单线程done，默认配置8个leveldb。

BenchMark

    ubuntu@VM-0-3-ubuntu:~/tool/redis-4.0.0/src$ ./redis-benchmark -h localhost -p 10086 -t set,get -c 10 -n 10000 -q
    SET: 14306.15 requests per second
    GET: 18315.02 requests per second

Environment

	腾讯云 1核 1GB 2Mbps
