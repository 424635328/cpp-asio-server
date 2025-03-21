#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include "http_request.h"
#include "http_response.h"

class HttpRequestHandler {
public:
    HttpResponse handle_request(const HttpRequest& request);
};

#endif