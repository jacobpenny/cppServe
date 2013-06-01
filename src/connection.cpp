#include "connection.h"
#include "request.h"

Connection::Connection(int f, std::string addr) : fd(f),
                                                  address(addr),
                                                  state(State::REQUEST),
                                                  bytes_sent(0),
                                                  head_bytes_sent(0),
                                                  head_sent(false),
                                                  request(new Request()) {}

void Connection::generate_response_head()
{
  // place-holder
  std::string size = std::to_string(request->resource_size());
  response_head = "HTTP/1.1 200 OK\r\n";
  response_head.append("Content-Length: ");
  response_head.append(size);
  response_head.append("\r\n");
  response_head.append("Connection: close\r\n\r\n");
}
