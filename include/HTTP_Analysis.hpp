#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <filesystem>
#include <fstream>

/*struct request_content
{
private:
    std::string _Host;
    std::string _User_Agent;
    std::string _Accept;
    std::string _Accept_Encoding;
    std::string Accept_Language;
    std::string Authorization;
    std::string Cookie;
    std::string Referer;
    std::string If_Modified_Since;
    std::string If_None_Match;
    std::string Range;
    std::string Origin;
    std::string Content_Type;
    std::string Content_Length;
    
public:
    request_content() = default;
};
*/
class HTTP_Analysis
{
public:
    using HTTP_SET = std::unordered_map<std::string, std::string>;
    using K_V = std::pair<std::string, std::string>;
private:
    HTTP_Analysis() = default;
    static HTTP_SET content_type;    // 存放响应内容主体的类型映射

    static HTTP_SET GET(const std::string &);
    static std::string RESPONSE(const HTTP_SET&);

public:
    HTTP_Analysis(const HTTP_Analysis &) = delete;
    HTTP_Analysis(HTTP_Analysis &&) = delete;

    static std::string package(const std::string &);

    ~HTTP_Analysis() = default;
};