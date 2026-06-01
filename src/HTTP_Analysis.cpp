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

std::string HTTP_Analysis::getline(std::string& data)
{
    int pos = data.find("\r\n");
    std::string line = data.substr(0, pos);
    data = data.substr(pos + 2, data.size() - pos - 1);
    return line;
}            

HTTP_Analysis::HTTP_SET HTTP_Analysis::GET(std::string &request)
{
    HTTP_SET new_request;
    std::string tmp;
    int ch = strlen("\r\n");
    // 请求行
    tmp = getline(request); //读取一整行，去除\r\n
    
    int pos = tmp.find(' ');
    new_request.emplace(K_V("method", tmp.substr(0, pos)));
    tmp = tmp.substr(pos + 1, tmp.size() - pos);
    
    pos = tmp.find(' ');
    new_request.emplace(K_V("url", tmp.substr(0, pos)));
    tmp = tmp.substr(pos + 1, tmp.size() - pos);
    
    pos = tmp.find(' ');
    new_request.emplace(K_V("version", tmp.substr(0, pos)));
    
    getline(request);   //去除请求行和首部行的"\r\n"
    
    //首部行
    // 定位到' : ',然后把前面的内容作为键，后面的内容作为值
    while(1)
    {
        tmp = getline(request);
        pos = tmp.find(':');
        if(pos == -1)
            break;
        new_request.emplace(K_V(tmp.substr(0, pos), tmp.substr(pos + 1, tmp.size() - pos)));
    }
    return new_request;
}

std::string HTTP_Analysis::RESPONSE(const HTTP_SET& request)
{
    //1.url资源存在 2.资源不存在
    auto URL = "/home/charon/www" + request.find("url")->second;     //可以再迭代
    if(std::filesystem::exists(URL) && std::filesystem::is_regular_file(URL))
    {
        std::ifstream ifs(URL, std::ios::binary | std::ios::in);    //消息主体必须传二进制
        //获取扩展名（然后转成Content-Type）
        size_t pos = URL.find_last_of('.');
        std::string file_type = URL.substr(pos + 1, URL.size() - pos);  //跳过'.'读取
        std::string _response = request.find("version")->second + " 200" + " OK\r\n" 
                                + "Content-Type: " + content_type.find(file_type)->second + "\r\n" 
                                + "Content-Length: " + std::to_string(std::filesystem::file_size(URL) - 1) + "\r\n" //减一是因为file_size可能计算了'\0' 
                                + "Connection: close\r\n" 
                                + "\r\n";

        //响应主体
        std::string tmp;
        while (std::getline(ifs, tmp))
        {
            _response += tmp;
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

std::string HTTP_Analysis::package(std::string& request) { return RESPONSE(std::move(GET(request))); }
