#include<iostream>
#include<string>
#include<jsoncpp/json/json.h>
int main()
{
    Json::Value root;
    root["name"]="zyq";
    root["high"]=1.58f;
    root["sex"]="女";

    //序列化
    std::string s=root.toStyledString();
    std::cout << s << std::endl;

    return 0;
}