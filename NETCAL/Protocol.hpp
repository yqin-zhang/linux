#pragma once

#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
class Request
{
    Request()
    {
    }
    // 序列化对象--结构体转字符串
    bool Serialize(std::string *out)
    {
        // 1.手写 "_x" "_open" "_y--字符串拼接转换
        // 2.现成工具
        Json::Value root;
        root["x"] = _x;
        root["y"] = _y;
        root["_oper"] = _oper;

        Json::StyledWriter writer;
        *out = writer.write(root);
        if (out->empty())
        {
            return false;
        }
        return true;
    }
    // 反序列化对象--字符串转结构体
    bool Deserialize(std::string &in)
    {
        Json::Value root;
        Json::Reader reader;
        bool ret = reader.parse(root);
        if (!ret)
        {
            return false;
        }
        _x = root["x"].asInt();
        _y = root["y"].asInt();
        _oper = root["oper"].asInt();

        return true;
    }
    ~Request()
    {
    }

private:
    // x oper y--约定1
    int _x;
    int _y;
    char _oper;
};

class Response
{
private:
    int result; // 10 / 0 = 0
    int _code;  // 0 1
};