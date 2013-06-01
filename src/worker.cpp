#include "worker.h"
#include "connection.h"
#include "poller.h"
#include "request.h"

#include <stdexcept>
#include <thread>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <unistd.h> // read
#include <sys/sendfile.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>

void Worker::operator()() {
  while (1) {
    Connection *c;
    queue_.wait_and_pop(c);

    switch (c->state)
    {
      case Connection::REQUEST:
        try {
          read_data(c);
          std::copy(c->request->raw_request->begin(),
                    c->request->raw_request->end(),
                    std::ostream_iterator<char>(std::cout));

          if (c->request->parse()) { // done reading and parsing request
            c->state = Connection::RESPONSE;
            c->generate_response_head(); 
            handle_request(c); 
          } else {
            poller_.rearm_read(c);
          }
        } catch (std::runtime_error& e) {
          std::cout << e.what() << std::endl << std::flush;
          poller_.remove(c);
        }
        break;
      
      case Connection::RESPONSE:
        try {
          handle_request(c);
        } catch (std::runtime_error& e) {
          std::cout << e.what() << std::endl << std::flush;
          poller_.remove(c);
        }
        break;
      
      default:
        assert(false);
        break;
    }
  }
}

void Worker::read_data(Connection* c) const
{
  int count;
  char buf[512];

  while (0 < (count = read(c->fd, buf, sizeof buf))) {
    std::copy(buf,
              buf + count,
              back_inserter(*(c->request->raw_request)));
  }

  if (-1 == count && errno != EAGAIN) { 
    throw std::runtime_error("read_data-read");
  } else if (0 == count) {
    throw std::runtime_error("read_data-read (client closed connection)");
  }
}

void Worker::handle_request(Connection* c) const
{
  switch (c->request->request_type) {
    case Request::STATIC:
      if (!c->head_sent) { 
        send_static_head(c, c->head_bytes_sent);
      }
      
      if (c->head_sent && c->request->is_get()) {
        if (send_static_file(c, c->bytes_sent)) {
          // finished sending response
          poller_.remove(c);
        } else {
          poller_.rearm_write(c);
        }
      } else {
        poller_.rearm_write(c);
      }
      break;
  }
}

bool Worker::send_static_file(Connection* c, size_t offset) const
{
  int resource_fd;
  std::string file_name(c->request->request_line[1]);
  
  if (-1 == (resource_fd = open(file_name.c_str() + 1, O_RDONLY))) { // +1 for leading slash
    throw std::runtime_error("send_static_file_open");
  } 
  
  off_t off = offset;
  int count = sendfile(c->fd,
                       resource_fd,
                       &off,
                       c->request->resource_size() - c->bytes_sent); 
  if (-1 == count && errno == EAGAIN) { // would block
    close(resource_fd);
    return false;
  } else if (-1 == count) {
    throw std::runtime_error("send_static_file-sendfile");
  }
  
  close(resource_fd);
  c->bytes_sent += count;
  return (c->bytes_sent == c->request->resource_size());
}

void Worker::send_static_head(Connection* c, size_t offset) const {
  int count = send(c->fd,
                   c->response_head.c_str() + offset,
                   c->response_head.size() - c->head_bytes_sent,
                   0);
  
  if (-1 == count && errno == EAGAIN) {
    return;
  } else if (-1 == count) {
    throw std::runtime_error("send_static_file-sendfile");
  }
  
  c->head_bytes_sent += count;
  if (c->head_bytes_sent == c->response_head.size()) {
    c->head_sent = true;
  }
}
