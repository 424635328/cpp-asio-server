// http_request_handler.h
#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include "http_request.h"
#include "http_response.h"

class HttpRequestHandler {
public:
    HttpResponse handle_request(const HttpRequest& request);
private:
    HttpResponse handle_static_file_request(const HttpRequest& request); // 添加静态文件处理函数声明
};

#endif