#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <vector>
#include <string>

class Connection {
  public:
    enum State { REQUEST, RESPONSE };

    int fd;
    State state;
    std::string address;
    std::vector<char> *buffer;

    Connection(int fd, std::string addr);
    ~Connection();
};

#endif

