#pragma once
#include<iostream>
#include<string>
#include<unordered_map>
#include"Logger.hpp"
class Dictionary
{
private:
     void LoadConf()
     {
        std::ifstream in(_path);
        if(!in.is_open())
        return ;

        std::string line;
        while(std::getline(in,line))
        {
            LOG(LogLevel::DEBUG)<<"load dict message"<<line;
        }
        in.close;
     }
public:
    Dictionary(const std::string &path):_path(path)
    {

    }
    std::string translate(const std::string &word,
        const std::string &whoip,uint16_t whoport)
    {

    }
    ~Dictionary()
    {

    }
private:
    std::string _path;
    std::unordered_map<std::string,std::string> _dict;

};