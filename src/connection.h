#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <vector>
#include <string>
#include <cstddef>

class Request;

class Connection {
  public:
    enum State { REQUEST, RESPONSE };

    int fd;
    std::string address;
    State state;
    size_t bytes_sent;
    size_t head_bytes_sent;
    bool head_sent;
    Request* request;
    std::string response_head;

    void generate_response_head();

    Connection(int fd, std::string addr);
    ~Connection() {}
};

#endif

