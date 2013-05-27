#include "connection.h"
#include "request.h"

Connection::Connection(int f, std::string addr)
: fd(f), address(addr), state(State::REQUEST), request(new Request()) {}



