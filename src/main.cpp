//
// Created by zgpan on 2018/6/24.
// modify 20181005

#include <iostream>
#include "server.h"
#include "mouse.h"
using namespace std;

//20180630 验证事件循环是否ok
void unit_test_server_case1()
{
    Server _server;
    _server.run(10086);
}


void unit_test_mouse_case()
{
   Mouse _server;
    _server.runMouseSvr();
}

int main()
{
    cout << "MouseKV" << std::endl;
    unit_test_mouse_case();
    return 0;
}
