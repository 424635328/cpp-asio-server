#include "asio_context.h"
#include "http_request_handler.h"
#include <iostream>

using namespace std;

// 处理 HTTP 请求，生成 HTTP 响应
HttpResponse HttpRequestHandler::handle_request(const HttpRequest& request) {
    cout << "收到请求: " << request.method << " " << request.uri << endl;
    HttpResponse response;
    response.status_code = 200;
    response.body = "<h1>Hello,World!</h1>"; // 返回 HTML 内容
    response.headers["Content-Type"] = "text/html";
    return response;
}