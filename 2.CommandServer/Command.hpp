#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <vector>

class Command
{
private:
    bool IsSafe(const std::string &cmd)
    {
        for (auto &c : _command_white_list)
        {
            if (cmd == c)
            {
                return true;
            }
        }
        return false;
    }

public:
    Command()
    {
        _command_white_list.push_back("ls -a -l");
        _command_white_list.push_back("ll");
        _command_white_list.push_back("tree");
        _command_white_list.push_back("pwd");
        _command_white_list.push_back("who");
        _command_white_list.push_back("whoami");
        _command_white_list.push_back("cat test.txt");
    }
    std::string Exec(const std::string &cmd)
    {
        if(!IsSafe(cmd))
        {
            return "坏人";
        }
        std::string result;
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp == NULL)
        {
            result = cmd + "exec error";
        }
        else
        {

            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), fp))
            {
                result += buffer;
            }

            pclose(fp);
        }
        return result;
    }
    ~Command()
    {
    }

private:
    std::vector<std::string> _command_white_list;
};