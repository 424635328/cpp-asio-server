//http_request_handler.h
#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include "http_request.h"
#include "http_response.h"
#include <string>

class HttpRequestHandler {
public:
    HttpResponse handle_request(const HttpRequest& request);

private:
    HttpResponse handle_static_file_request(const HttpRequest& request);
    std::string url_decode(const std::string& str);
    HttpResponse analyze_behavior(const HttpRequest& request);  //行为分析函数
    std::unordered_map<std::string, std::string> parse_post_data(const std::string& body);
};

#endif