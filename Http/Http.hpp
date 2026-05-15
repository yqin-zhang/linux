#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include "Logger.hpp"
static const std::string linesep = "\r\n";
static const std::string linesep1 = " ";
static const std::string linesep2 = ": ";
static const std::string defaulthome = "index.html";
static const std::string webroot = "./wwwroot";
static const std::string html_404 = "404.html";
static const std::string suffixsep = ".";
using namespace LogModule;
// 定制HTTP协议
class HttpRequest
{
private:
    std::string ReadOneLine(std::string &reqstr, bool *status)
    {
        auto pos = reqstr.find(linesep); // 返回找到的下标
        if (pos == std::string::npos)
        {
            *status = false;
            return std::string();
        }
        *status = true;
        // 到这里至少有一个报文
        auto line = reqstr.substr(0, pos);
        reqstr.erase(0, pos + linesep.size());
        return line;
    }
    void ParaseReqLine(std::string &reqline)
    {
        std::stringstream ss(reqline);
        // 空格就作为了分隔符
        ss >> _mothod >> _uri >> _httpversion;
    }
    void BuildKV(std::string &line, std::string *k, std::string *v)
    {
        auto pos = line.find(linesep2);
        if (pos == std::string::npos)
        {
            *k = *v = std::string();
            return;
        }
        *k = line.substr(0, pos);
        *v = line.substr(pos + linesep2.size());
    }

public:
    HttpRequest()
    {
    }
    void Serialize()
    {
        // 不做
    }
    // 接收
    // reqstr:一定是一个完整的HTTP请求字符串
    bool Deserialize(std::string &reqstr)
    {
        bool status = true;
        std::string reqline = ReadOneLine(reqstr, &status);
        if (!status)
        {
            return false;
        }
        std::cout << "#############" << std::endl;
        std::cout << reqline;
        std::cout << "#############" << std::endl;

        ParaseReqLine(reqline);

        // LOG(LogLevel::DEBUG) << "_mothod:" << _mothod;
        // LOG(LogLevel::DEBUG) << "_uri:" << _uri;
        // LOG(LogLevel::DEBUG) << "_httpversion:" << _httpversion;

        while (1)
        {
            status = true;
            reqline = ReadOneLine(reqstr, &status);
            if (status && !reqline.empty())
            {
                std::string k, v;
                BuildKV(reqline, &k, &v);
                if (k.empty() || v.empty())
                {
                    continue;
                }
                _req_headers.insert(std::make_pair(k, v));
            }
            else if (status) // 空行
            {
                _blank_line = linesep;
                break;
            }
            else
            {
                // return false;
                break;
            }
        }

        _rep_body = reqstr;

        _path = webroot;
        _path += _uri; //./wwwroot/
        if (_uri == "/")
        {

            _path += defaulthome; //./wwwroot/index.html
        }
        LOG(LogLevel::DEBUG) << "_path:" << _path;
        return true;
    }
    std::string Path()
    {
        return _path;
    }
    void SetPath(const std::string &path)
    {
        _path = path;
    }
    std::string Suffix()
    {
        //_path: /index.html
        if (_path.empty())
        {
            return std::string();
        }
        else
        {
            auto pos = _path.rfind(suffixsep);
            if (pos == std::string::npos)
            {
                return std::string();
            }
            else
            {
                return _path.substr(pos); // .html
            }
        }
    }

    ~HttpRequest()
    {
    }

private:
    std::string _mothod;
    std::string _uri;
    std::string _httpversion; // http版本
    std::unordered_map<std::string, std::string> _req_headers;
    std::string _blank_line; // 空行
    std::string _rep_body;   // 请求正文

    std::string _path; // 我们真正的要访问的资源的路径
};

class HttpResponse
{
private:
    std::string Code2Desc(int code)
    {
        switch (code)
        {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        default:
            return "";
        }
    }

public:
    HttpResponse()
        : _httpversion("HTTP/1.1"),
          _blank_line("\r\n")
    {
    }
    std::string Serialize()
    {
        std::stringstream ss;
        ss << _httpversion << " " << _code << " " << _desc << linesep;

        if (!_resp_body.empty())
        {
            std::string len = std::to_string(_resp_body.size());
            SetHeader("Content-Length", len);
        }

        for (auto &kv : _resp_handers)
            ss << kv.first << ": " << kv.second << linesep;
        ss << linesep;

        ss << _resp_body;
        return ss.str();
    }

    void Deserialize(std::string &reqstr)
    {
        // 不做
    }

    bool ReadContent(const std::string &path)
    {
        // 以二进制方式读取
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            _resp_body = "";
            LOG(LogLevel::WARNING) << path << "资源不存在";
            return false;
        }

        std::stringstream ss;
        ss << file.rdbuf();
        _resp_body = ss.str();
        file.close();
        return true;
    }

    void SetCode(int code)
    {
        if (code >= 100 && code < 600)
        {

            _code = code;             // 状态码
            _desc = Code2Desc(_code); // 转化为状态码描述
        }
        else
        {
            LOG(LogLevel::DEBUG) << "非法的状态码" << code;
        }
    }
    void SetHeader(const std::string &key, const std::string &value)
    {
        _resp_handers[key] = value;
    }

    ~HttpResponse()
    {
    }

private:
    std::string _httpversion;
    int _code;
    std::string _desc;
    std::unordered_map<std::string, std::string> _resp_handers;
    std::string _blank_line;
    std::string _resp_body;

    std::string _path;
};

class Http
{
private:
    std::string Suffix2Desc(const std::string &suffix)
    {
        if (suffix == ".html")
            return "text/html";
        else if (suffix == ".css")
            return "text/css";
        else if (suffix == ".js")
            return "application/x-javascript";
        else if (suffix == ".png")
            return "image/png";
        else if (suffix == ".jpg")
            return "image/jpeg";
        else if (suffix == ".txt")
            return "text/plain";
    }

public:
    Http()
    {
    }
    std::string HandlerRequst(std::string &requeststr)
    {
        std::string respstr;
        HttpRequest req;
        std::cout<<requeststr<<std::endl;
        // 反序列化

        if (req.Deserialize(requeststr))
        {

            HttpResponse resp;
            if (resp.ReadContent(req.Path()))
            {
                std::string suffix = req.Suffix();
                std::string mime_type_value = Suffix2Desc(suffix); // 资源后缀，转成Content-type
                resp.SetHeader("Content-Type", mime_type_value);
                resp.SetCode(200);
            }
            else
            {
                // 真实站点是如何访问的
                // 前后端配合
                //  ./wwwroot/404.html
                std::string err_404 = webroot + "/" + html_404;
                req.SetPath(err_404);
                resp.ReadContent(err_404);
                std::string suffix = req.Suffix();
                std::string mime_type_value = Suffix2Desc(suffix);
                resp.SetHeader("Content-Type", mime_type_value);
                // 资源不存在
                resp.SetCode(404);
            }

            respstr = resp.Serialize();
        }
        return respstr;
    }
    ~Http()
    {
    }
};