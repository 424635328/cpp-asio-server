#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include "http_request.h"
#include "http_response.h"

using namespace std; 

// HttpRequestHandler 类，处理 HTTP 请求并返回 HTTP 响应
class HttpRequestHandler {
public:
    // 处理 HTTP 请求，返回对应的 HTTP 响应
    virtual HttpResponse handle_request(const HttpRequest& request);
};

#endif