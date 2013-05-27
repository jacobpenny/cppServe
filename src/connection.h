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

    Request* request;

    Connection(int fd, std::string addr);
    ~Connection() {}
};

#endif

