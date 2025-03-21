#include "http_request.h"
#include <sstream>
#include <iostream>

std::istream& operator>>(std::istream& is, HttpRequest& request) {
    std::string line;

    // Read the request line
    std::getline(is, line);
    std::istringstream request_line(line);
    request_line >> request.method >> request.uri >> request.version;

    std::cout << "HttpRequest::operator>> - Request line: " << line << std::endl;
    std::cout << "HttpRequest::operator>> - Method: " << request.method << ", URI: " << request.uri << ", Version: " << request.version << std::endl;

    // Read headers
    while (std::getline(is, line) && line != "\r") { // Stop at the empty line after headers
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 2); // Skip ": "
            request.headers[header_name] = header_value;
            std::cout << "HttpRequest::operator>> - Header: " << header_name << ": " << header_value << std::endl;
        }
    }
    std::cout << "HttpRequest::operator>> - End of headers." << std::endl;

    //Read the body.  (Simple implementation, reads until the end of the stream)
    std::stringstream body_stream;
    body_stream << is.rdbuf();
    request.body = body_stream.str();
    std::cout << "HttpRequest::operator>> - Body: " << request.body << std::endl;
    return is;
}