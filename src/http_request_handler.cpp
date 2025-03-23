#include "asio_context.h"
#include "http_request_handler.h"
#include "http_request.h"
#include "http_response.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>  // transform
#include <iomanip>   // quoted

using namespace std;

// Helper function to URL decode a string
std::string HttpRequestHandler::url_decode(const std::string &str)
 {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int hex_val;
            std::stringstream ss;
            ss << std::hex << str.substr(i + 1, 2);
            ss >> hex_val;
            result += static_cast<char>(hex_val);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::unordered_map<std::string, std::string> HttpRequestHandler::parse_post_data(const std::string& body) {  // 添加 HttpRequestHandler::
    std::unordered_map<std::string, std::string> data;
    std::stringstream ss(body);
    std::string pair, key, value;

    while (std::getline(ss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            key = pair.substr(0, pos);
            value = pair.substr(pos + 1);
            data[url_decode(key)] = url_decode(value); // 解码并存储键值对
        }
    }
    return data;
}

// Serve static files
HttpResponse HttpRequestHandler::handle_static_file_request(const HttpRequest& request) {
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



// 行为分析（简化示例）
HttpResponse HttpRequestHandler::analyze_behavior(const HttpRequest& request) {
    HttpResponse response;

    // 检查 User-Agent 是否为空或异常
    if (request.headers.find("User-Agent") == request.headers.end() || request.headers.at("User-Agent").empty()) {
        std::cerr << "Suspicious request: Missing User Agent header." << std::endl;
        response.status_code = 403; // Forbidden
        response.body = "<h1>403 Forbidden - Missing User-Agent</h1>";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        return response;
    }

    // TODO: 更复杂的行为分析，例如：
    // 1. 检查请求频率
    // 2. 检查请求的 URI 模式
    // 3. 检查是否存在已知的恶意 User-Agent

    return response; // 默认情况下，返回空响应，表示未检测到异常行为
}

HttpResponse HttpRequestHandler::handle_request(const HttpRequest& request) {
    cout << "收到请求: " << request.method << " " << request.uri << endl;
    // 应用层防御：行为分析
    HttpResponse behavior_response = analyze_behavior(request);
    if (behavior_response.status_code != 0) {
        return behavior_response; // 如果行为分析检测到异常，则返回错误响应
    }

    if (request.uri == "/contact" && request.method == "POST") {
        auto post_data = parse_post_data(request.body);

        std::string name = post_data["name"];
        std::string email = post_data["email"];
        std::string message = post_data["message"];

        if (name.empty() || email.empty() || message.empty()) {
            std::cerr << "Error: Contact form submission with missing fields." << std::endl;
            HttpResponse response;
            response.status_code = 400;
            response.headers["Content-Type"] = "text/html; charset=utf-8";
            response.body = R"(<!DOCTYPE html><html lang='zh-CN'><head><meta charset='UTF-8'><title>错误</title></head><body><h1>错误</h1><p>请填写所有字段！</p><a href='/web/contact.html'>返回</a><script src="/web/beautify.js"></script></body></html>)";

            return response;
        }

        std::cout << "Contact Form Submission:" << std::endl;
        std::cout << "Name: " << std::quoted(name) << std::endl;
        std::cout << "Email: " << std::quoted(email) << std::endl;
        std::cout << "Message: " << std::quoted(message) << std::endl;

        HttpResponse response;
        response.status_code = 200;
        response.version = "HTTP/1.1";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        response.body = R"(<!DOCTYPE html><html lang='zh-CN'><head><meta charset='UTF-8'><title>错误</title></head><body><h1>错误</h1><p>请填写所有字段！</p><a href='/web/contact.html'>返回</a><script src="/web/beautify.js"></script></body></html>)";
        return response;

    } else {
        return handle_static_file_request(request);
    }
}