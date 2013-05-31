#include "poller.h"
#include "acceptor.h"
#include "connection.h"

#include <stdexcept>
#include <cassert>
#include <iostream>
#include <sys/epoll.h>
#include <stdlib.h> // calloc
#include <unistd.h> // close, read, write
#include <string.h> // strerr

#define MAXEVENTS 64

Poller::Poller(const Acceptor& a, threadsafe_queue<Connection*>& queue) 
: acceptor_(a), work_queue_(queue)
{
  if (-1 == (epoll_fd_ = epoll_create1(0))) {
    throw std::runtime_error("epoll_create1");
  }
  auto c = new Connection(acceptor_.get_listen_fd(), "localhost");
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET;
  event.data.ptr = c;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl");
  }
}

Poller::~Poller()
{
}

void Poller::start() 
{
  struct epoll_event *events;
  struct epoll_event event;
  events = (epoll_event*) calloc(MAXEVENTS, sizeof event);

  while (1) {
    int numEvents = epoll_wait(epoll_fd_, events, MAXEVENTS, -1);
    for (int i = 0; i < numEvents; i++) {
      auto c = (Connection*) events[i].data.ptr;
      int event_fd = c->fd;

      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
        assert(event_fd != acceptor_.get_listen_fd());
        remove(c);
      } else if (event_fd == acceptor_.get_listen_fd()) {
        listen_fd_handler();
      } else if (events[i].events & EPOLLIN && c->state == Connection::REQUEST) {
        work_queue_.push(c);
      } else if (events[i].events & EPOLLOUT && c->state == Connection::RESPONSE) {
        work_queue_.push(c);
      } else {
        std::cout << "What happens here..." << std::endl;
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

void Poller::add_to_read_poll(Connection *c)
{
  static struct epoll_event event;
  event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
  event.data.ptr = c;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl add to read poll");
  }
}

void Poller::remove_from_read_poll(Connection *c)
{
  static struct epoll_event event;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl remove from read poll");
  }
}

void Poller::rearm_read(Connection *c)
{
  static struct epoll_event event;
  event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
  event.data.ptr = c;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl rearm read");
  } 
}

void Poller::add_to_write_poll(Connection *c)
{
  static struct epoll_event event;
  event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
  event.data.ptr = c;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl add to write poll");
  }
}

void Poller::remove_from_write_poll(Connection *c)
{
  remove_from_read_poll(c);
}

void Poller::rearm_write(Connection *c)
{
  static struct epoll_event event;
  event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
  event.data.ptr = c;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl rearm write");
  } 
}

void Poller::switch_to_write_poll(Connection *c)
{
  static struct epoll_event event;
  event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
  event.data.ptr = c;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl switch write");
  } 
}

void Poller::remove(Connection *c)
{
  static struct epoll_event event;
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, c->fd, &event)) {
    throw std::runtime_error("epoll_ctl remove");
  }
  if (-1 == close(c->fd)) {
    std::cout << strerror(errno) << std::endl;
  }
  delete c;
}
