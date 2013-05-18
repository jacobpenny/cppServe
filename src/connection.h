#ifndef CONNECTION_H
#define CONNECTION_H

#include <vector>
#include <string>

class Connection {
  public:
    int fd;
    std::vector<char> *buffer;
    std::string address;

    Connection();
    ~Connection();
};

Connection::Connection() : buffer(new std::vector<char>(1024)) {}
Connection::~Connection() { delete buffer; }



#endif

