#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include "http_request.h"
#include "http_response.h"

class HttpRequestHandler {
public:
    virtual HttpResponse handle_request(const HttpRequest& request);
};

#endif