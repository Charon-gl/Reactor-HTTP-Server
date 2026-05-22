#include "HTTP_Analysis.hpp"

HTTP_Analysis::HTTP_SET HTTP_Analysis::content_type = {
    {"html", "text/html; charset=utf-8"},
    {"htm", "text/html; charset=utf-8"},
    {"txt", "text/plain"},
    {"css", "text/css; charset=utf-8"}, 
    {"js", "application/javascript; charset=utf-8"},
    {"png", "image/png"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"svg", "image/svg+xml"},
    {"ico", "image/x-icon"},
    {"json", "application/json"}
};

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
    // 定位到' : ',然后把前面的内容作为键，后面的内容作为值
    while(std::getline(req, tmp))
    {
        if(tmp == "\r\n")
            break;
        pos = tmp.find(':');
        new_request.emplace(K_V(tmp.substr(0, pos), tmp.substr(pos + 1, tmp.size() - pos)));
        tmp.clear();
    }
    return new_request;
}

std::string HTTP_Analysis::RESPONSE(const HTTP_SET& request)
{
    //1.url资源存在 2.资源不存在
    auto URL = "/home/admin/www" + request.find("url")->second;     //可以再迭代
    if(std::filesystem::exists(URL) && std::filesystem::is_regular_file(URL))
    {
        std::ifstream ifs(URL, std::ios::binary | std::ios::in);    //消息主体必须传二进制
        //获取扩展名（然后转成Content-Type）
        size_t pos = URL.find_last_of('.');
        std::string file_type = URL.substr(pos + 1, URL.size() - pos);  //跳过'.'读取
        std::string _response = request.find("version")->second + " 200" + " OK\r\n" 
                                + "Content-Type: " + content_type.find(file_type)->second + "\r\n" 
                                + "Content-Length: " + std::to_string(std::filesystem::file_size(URL)) + "\r\n" 
                                + "Connection: close\r\n" 
                                + "\r\n";

        //响应主体
        std::string tmp;
        while (std::getline(ifs, tmp))
        {
            _response += tmp;
            tmp.clear();
        }
        ifs.close();
        return _response;
    }
    //简单处理，默认返回404
    std::string no_resource = "<h1>404 Not Found</h1>";
    std::string _response = request.find("version")->second + " 404" + " NotFound\r\n" 
                            + "Content-Type: " + "text/html\r\n" 
                            + "Content-Length: " + std::to_string(no_resource.size() )+ "\r\n" 
                            + "Connection: close\r\n" 
                            + "\r\n" 
                            + no_resource;
    return _response;
}

std::string HTTP_Analysis::package(const std::string& request) { return RESPONSE(std::move(GET(request))); }
