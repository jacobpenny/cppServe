#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <vector>
#include <string>
#include <cstddef>

class Connection {
  public:
    enum State { REQUEST, RESPONSE };
    enum RequestType { STATIC, CGI };

    int fd;
    State state;
    std::string address;
    std::vector<char> *buffer;
    size_t writeMarker;
    RequestType type;

    Connection(int fd, std::string addr);
    ~Connection();
};

#endif

