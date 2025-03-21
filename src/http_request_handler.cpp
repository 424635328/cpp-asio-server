#include "asio_context.h"
#include "http_request_handler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map> // 引入 unordered_map

using namespace std;

// 处理 HTTP 请求，生成 HTTP 响应
HttpResponse HttpRequestHandler::handle_request(const HttpRequest& request) {
    cout << "收到请求: " << request.method << " " << request.uri << endl;
    HttpResponse response;
    response.status_code = 200;

    // 维护一个 MIME 类型映射表
    static const unordered_map<string, string> mime_types = {
        {".html", "text/html; charset=utf-8"}, // 添加charset
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".webp", "image/webp"}, //添加webp格式支持
        {".svg", "image/svg+xml"},
        {".txt", "text/plain"},
    };

    // 构建静态文件路径
    string file_path = request.uri;
    if (file_path == "/") {
        file_path = "../web/index.html"; // 默认访问 index.html
    }

    // 检查文件是否存在
    ifstream file(file_path.substr(1), ios::binary); // 去掉开头的 "/"
    if (file.is_open()) {
        stringstream file_content;
        file_content << file.rdbuf();
        response.body = file_content.str();
        file.close();

        // 获取文件扩展名
        size_t dot_pos = file_path.rfind('.');
        string file_ext = (dot_pos == string::npos) ? "" : file_path.substr(dot_pos);

        // 设置 Content-Type
        auto mime_it = mime_types.find(file_ext);
        if (mime_it != mime_types.end()) {
            response.headers["Content-Type"] = mime_it->second;
        } else {
            response.headers["Content-Type"] = "application/octet-stream"; // 默认二进制流
        }

        // 设置缓存控制
        response.headers["Cache-Control"] = "max-age=3600";

    } else {
        // 文件未找到，返回 404
        response.status_code = 404;
        response.body = "<h1>404 Not Found</h1>";
        response.headers["Content-Type"] = "text/html; charset=utf-8"; // 添加charset
    }

    return response;
}