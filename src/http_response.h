#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <unordered_map>

class HttpResponse {
public:
    int status_code;
    std::string version = "HTTP/1.1";
    std::unordered_map<std::string, std::string> headers; // Remove const
    std::string body;

    std::string to_string() const;
};

#endif