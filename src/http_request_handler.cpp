#include "asio_context.h"
#include "http_request_handler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace std;

HttpResponse HttpRequestHandler::handle_request(const HttpRequest& request) {
    cout << "收到请求: " << request.method << " " << request.uri << endl;
    HttpResponse response;
    response.status_code = 200;

    static const unordered_map<string, string> mime_types = {
        {".html", "text/html; charset=utf-8"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".webp", "image/webp"},
        {".svg", "image/svg+xml"},
        {".txt", "text/plain"},
    };

    string file_path = request.uri;
    if (file_path == "/") {
        file_path = "../web/index.html";
    }

    ifstream file(file_path.substr(1), ios::binary);
    if (file.is_open()) {

        stringstream file_content;
        file_content << file.rdbuf(); // 使用 stringstream 一次性读取文件内容
        response.body = file_content.str();  // 将文件内容存储在 response.body 中
        file.close();

        size_t dot_pos = file_path.rfind('.');
        string file_ext = (dot_pos == string::npos) ? "" : file_path.substr(dot_pos);

        auto mime_it = mime_types.find(file_ext);
        if (mime_it != mime_types.end()) {
            response.headers["Content-Type"] = mime_it->second;
        } else {
            response.headers["Content-Type"] = "application/octet-stream";
        }
        response.headers["Cache-Control"] = "max-age=3600";

    } else {
        response.status_code = 404;
        response.body = "<h1>404 Not Found</h1>";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
    }

    return response;
}