#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>
#include <istream>

class HttpRequest {
public:
    std::string method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    friend std::istream& operator>>(std::istream& is, HttpRequest& request);
};

#endif