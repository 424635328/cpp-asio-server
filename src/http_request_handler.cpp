#include "http_request_handler.h"
#include <iostream>

HttpResponse HttpRequestHandler::handle_request(const HttpRequest& request) {
  std::cout << "Received request: " << request.method << " " << request.uri << std::endl;
  HttpResponse response;
  response.status_code = 200;
  response.body = "<h1>Hello, World!</h1>";
  response.headers["Content-Type"] = "text/html";
  return response;
}