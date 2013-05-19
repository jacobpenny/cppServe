#include "worker.h"
#include "connection.h"
#include "poller.h"

#include <stdexcept>
#include <thread>
#include <iostream>
#include <unistd.h>

void Worker::operator()() {
  while (1) {
    Connection *connection;
    queue_.wait_and_pop(connection);
    std::cout << "Thread " << std::this_thread::get_id() << std::endl;
    read_data(connection);
  }
}

void Worker::read_data(Connection* connection) const {
  int count;
  char buf[512];

  while (0 < (count = read(connection->fd, buf, sizeof buf))) {
    if (-1 == (write(1, buf, count))) {
      throw std::runtime_error("read");
    }
  }

  if (-1 == count && EAGAIN != errno) {
    throw std::runtime_error("read");
  } else if (count == 0) {
    std::cout << "Connection closed." << std::endl;
    poller_.remove(connection);
    // The remote has closed the connection.
    // TODO what happens here with keep-alive connections?
  }
}
