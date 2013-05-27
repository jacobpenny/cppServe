#include "worker.h"
#include "connection.h"
#include "poller.h"
#include "request.h"

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
    
    switch (c->state)
    {
      case Connection::REQUEST:
        try {
          read_data(c);
          std::copy(c->request->raw_request->begin(), c->request->raw_request->end(), std::ostream_iterator<char>(std::cout));
          if (c->request->parse()) { // done reading and parsing request
            c->state = Connection::RESPONSE;
            poller_.remove_from_read_poll(c); // put these into parse request?
            poller_.add_to_write_poll(c);
            handle_request(c); // will return immediately if write isn't ready
          } else {
            poller_.rearm_read(c);
          }
        } catch (std::runtime_error& e) {
          std::cout << e.what() << std::endl << std::flush;
          poller_.remove_from_read_poll(c);
          delete c;
        }
        break;
      case Connection::RESPONSE:
        try {
          handle_request(c);
          std::cout << "Response" << std::endl;
        } catch (std::runtime_error& e) {
          std::cout << e.what() << std::endl << std::flush;
        }
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
    std::copy(buf, buf + count, back_inserter(*(c->request->raw_request)));
  }
  
  if (-1 == count && EAGAIN != errno) {
    throw std::runtime_error("read");
  } else if (count == 0) {
    throw std::runtime_error("User closed connection");
  }
}

void Worker::handle_request(Connection* c) const
{
  int count = write(c->fd, &*(c->request->raw_request->begin() + c->request->write_marker), c->request->raw_request->size() - c->request->write_marker);
  c->request->write_marker += count;
  std::cout << count << std::endl << std::flush;
  poller_.remove(c);
  
  switch (c->request->request_type) {
    case Request::STATIC:
      std::string file_name(c->request->request_line[1]);
      ifstream file;
      file.open(std::string(file_name.begin() + 1, file_name.end()));
      

    break;
  }

  /* pseudo
    if (static resource)
      if (not in cache)
        add to cache, set byteSent to 0
      send from cache, keeping track of bytes sent for subsequent calls      
    else if (cgi)
      store cgi output in connection->writebuffer, send what we can, keeping track of bytes sent
    end
  */

}

