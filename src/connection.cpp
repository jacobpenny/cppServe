#include "connection.h"

Connection::Connection(int f, std::string addr) 
  : fd(f), state(State::REQUEST), address(addr), buffer(new std::vector<char>(1024)) {}

Connection::~Connection() { delete buffer; }


