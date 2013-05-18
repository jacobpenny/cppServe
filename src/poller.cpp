#include "poller.h"
#include "acceptor.h"
#include "connection.h"

#include <stdexcept>
#include <cassert>
#include <iostream>
#include <sys/epoll.h>
#include <stdlib.h> // calloc
#include <unistd.h> // close, read, write

#define MAXEVENTS 64

Poller::Poller(const Acceptor& a) : acceptor_(a) 
{
  if (-1 == (epoll_fd_ = epoll_create1(0))) {
    throw std::runtime_error("epoll_create1");
  }
  add_to_read_poll(new Connection(acceptor_.get_listen_fd(), "localhost"));
}

Poller::~Poller()
{
}

void Poller::start() 
{
  struct epoll_event *events;
  struct epoll_event event;
  events = (epoll_event*) calloc(MAXEVENTS, sizeof event);

  // event loop
  while (1) {
    int numEvents = epoll_wait(epoll_fd_, events, MAXEVENTS, -1);
    for (int i = 0; i < numEvents; i++) {
      auto connection_info = (Connection*) events[i].data.ptr;
      int event_fd = connection_info->fd;

      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
        // An error has occured on this fd
        assert(event_fd != acceptor_.get_listen_fd());
        close(event_fd); // will likely attach more data...
        if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event_fd, &events[i])) {
          std::cout << "epoll_ctl error in start()" << std::endl << std::flush;
        }
      } else if (event_fd == acceptor_.get_listen_fd()) {
        // Incoming connection(s) on the listen socket
        listen_fd_handler();
      } else if (events[i].events & EPOLLIN) {
        // A socket is ready to read
        print_data(event_fd); // temporary
      }
    }
  }
}

// This method handles events on the listen socket, which could one or more waiting connections
void Poller::listen_fd_handler()
{
  try { 
    std::vector<Connection*> connections = acceptor_.accept_connections();
    for(auto i : connections) {
      add_to_read_poll(i);
    }
  } catch (std::runtime_error& e) {
    std::cout << e.what();
  }
}

void Poller::add_to_read_poll(Connection *connection)
{
  static struct epoll_event event;
  event.events = EPOLLIN | EPOLLET;

  event.data.ptr = connection;

  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, connection->fd, &event)) {
    throw std::runtime_error("epoll_ctl");
  }
}

void Poller::print_data(int fd) const {
  int count;
  char buf[512];

  while (0 < (count = read(fd, buf, sizeof buf))) {
    if (-1 == (write(1, buf, count))) {
      throw std::runtime_error("read");
    }
  }

  if (-1 == count && EAGAIN != errno) {
    throw std::runtime_error("read");
  } else if (count == 0) {
    // The remote has closed the connection.
    // TODO what happens here with keep-alive connections?
  }
}
