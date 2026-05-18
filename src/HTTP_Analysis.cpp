#include "HTTP_Analysis.hpp"

std::string HTTP_Analysis::trim(const std::string& _data)
{
    
}

HTTP_Analysis::HTTP_SET HTTP_Analysis::GET(const std::string &request)
{
    HTTP_SET new_request;
    std::istringstream req(request);    //将request转换成输入流,再用getline读取一整行
    std::string tmp;
    
    //请求行
    std::getline(req, tmp);
    int pos = tmp.find(' ');
    new_request.emplace(K_V("method", tmp.substr(0, pos)));

    tmp = tmp.substr(pos + 1, tmp.size() - pos);
    pos = tmp.find(' ');
    new_request.emplace(K_V("url", tmp.substr(0, pos)));

    tmp = tmp.substr(pos + 1, tmp.size() - pos);
    pos = tmp.find(' ');
    new_request.emplace(K_V("version", tmp.substr(0, pos)));

    //首部行
    // 定位到' : ',然后把前面的内容作为键，后面的内容作为值（自定义一个清洗函数）
    while(std::getline(req, tmp))
    {
        if(tmp == "\r\n")
            break;
        pos = tmp.find(':');
        //考虑使用一个自定义的trim去掉空格
        new_request.emplace(K_V(tmp.substr(0, pos), tmp.substr(pos + 1, tmp.size() - pos)));
    }
    return new_request;
}

std::string HTTP_Analysis::RESPONSE(HTTP_SET obj)
{
    //需要分情况：1.url资源存在 2.资源不存在


}

void HTTP_Analysis::package(const std::string& request)
{
    HTTP_SET obj = std::move(GET(request));
    std::string response = RESPONSE(obj);
}
