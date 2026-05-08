#pragma once
#include<iostream>
#include<string>
#include<unordered_map>
#include"Logger.hpp"

static const std::string sep=" :";
class Dictionary
{
private:
     void LoadConf()
     {
        std::ifstream in(_path);
        if(!in.is_open())
        {
            LOG(LogLevel::ERROR)<<"open file error:"<<_path;
            return ;
        }


        std::string line;
        while(std::getline(in,line))
        {
            LOG(LogLevel::DEBUG)<<"load dict message"<<line;
            //dog:狗
            auto pos =line.find(sep);
            if(pos==std::string::npos)
            {
                LOG(LogLevel::WARNING)<<"format error"<<line;
                continue;
            }
            std::string word=line.substr(0,pos);//[)
            std::string value=line.substr(pos+sep.size());
            if(word.empty()||value.empty())
            {
                LOG(LogLevel::WARNING)<<"foemat error,word or value is empty"<<line;
            }

            _dict.insert(std::make_pair(word,value));
        }
        in.close();
     }
public:
    Dictionary(const std::string &path):_path(path)
    {
        LOG(LogLevel::INFO)<<"construct Dictionary obj";
        LoadConf();
    }
    std::string translate(const std::string &word,
        const std::string &whoip,uint16_t whoport)
    {
        (void)whoip;
        (void)whoport;
        auto iter=_dict.find(word);
        if(iter==_dict.end())
        {
            return "unknown";
        }
        return iter->first +"->"+iter->second;
    }
    ~Dictionary()
    {

    }
private:
    std::string _path;
    std::unordered_map<std::string,std::string> _dict;

};