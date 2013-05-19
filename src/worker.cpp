#include "worker.h"
#include "connection.h"
#include "poller.h"

#include <stdexcept>
#include <thread>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <unistd.h>

void Worker::operator()() {
  while (1) {
    Connection *c;
    queue_.wait_and_pop(c);
    
    std::cout << "Thread " << std::this_thread::get_id() << std::endl;
    switch (c->state)
    {
      case Connection::REQUEST:
        try {
          read_data(c);
          std::copy(c->buffer->begin(), c->buffer->end(), std::ostream_iterator<char>(std::cout));
          
          /*
          if (parse_request(c)) {
            // request received and parsed
          }
          */
        
        } catch (std::runtime_error& e) {
          std::cout << e.what() << std::endl << std::flush;
          poller_.remove(c);
        }
        break;
      case Connection::RESPONSE:

        break;
      default:
        throw std::runtime_error("unknown state");
    }
  }
}

void Worker::read_data(Connection* c) const
{
  int count;
  char buf[512];

  while (0 < (count = read(c->fd, buf, sizeof buf))) {
    std::copy(buf, buf + count, back_inserter(*(c->buffer)));
  }
  
  if (-1 == count && EAGAIN != errno) {
    throw std::runtime_error("read");
  } else if (count == 0) {
    throw std::runtime_error("User closed connection");
  }
}


