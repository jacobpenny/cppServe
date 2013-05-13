#include "connectionpoller.h"
#include "connection.h"
#include "utils.h"

#include <stdexcept>
#include <cassert>
#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXEVENTS 64

ConnectionPoller::ConnectionPoller(int listen_port) : listen_port_(listen_port) 
{
  epoll_fd_ = epoll_create1(0);
  if (-1 == epoll_fd_) {
    throw std::runtime_error("epoll_create1");
  }

  try {
    init_listen_fd();
  } catch (std::exception &e) {
    std::cout << e.what();
  }
}

ConnectionPoller::~ConnectionPoller()
{
  // close listen_fd_, close epoll instance
}

void ConnectionPoller::start() 
{
  struct epoll_event *events;
  struct epoll_event event;
  events = (epoll_event*) calloc(MAXEVENTS, sizeof event);

  // event loop
  while (1) {
    int numEvents = epoll_wait(epoll_fd_, events, MAXEVENTS, -1);
    for (int i = 0; i < numEvents; i++) {

      auto connection_info = (Connection *) events[i].data.ptr;
      int event_fd = connection_info->fd;

      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
        // An error has occured on this fd, close the socket and remove from epoll instance
        assert(event_fd != listen_fd_);
        close(event_fd); // will likely attach more data...
        if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event_fd, &events[i])) {
          std::cout << "epoll_ctl error in start()" << std::endl << std::flush;
        }
      } else if (event_fd == listen_fd_) {
        // Incoming connection(s) on the listen socket
        listen_fd_handler(events[i]);
      } else if (events[i].events & EPOLLIN) {
        // A socket is ready to read
        // this is temporary, just prints the request out. need to get notify worker pool here
        int count = 0;
        char buf[512];

        while (0 < (count = read(event_fd, buf, sizeof buf))) {
          /* Write the buffer to standard output */
          int s = write(1, buf, count);
          if (s == -1) {
            abort();
          }
        }

        if (-1 == count && EAGAIN != errno) {
          // There has been an unexpected error, throw
          throw std::runtime_error("read");
        } else if (count == 0) {
          /* End of file. The remote has closed the
             connection. */
          // close socket and remove from epoll_fd_
          break;
        }
      }
    }
  }
}

// This method handles events on the listen socket, which could one or more waiting connections
void ConnectionPoller::listen_fd_handler(struct epoll_event &ev)
{
  static struct sockaddr_storage client_addr;
  static socklen_t sin_size;
  static int client_fd;

  sin_size = sizeof(client_addr);

  while (-1 != (client_fd = accept(listen_fd_, (struct sockaddr *)&client_addr, &sin_size))) {
    // No error, time to accept connection and add to epoll instance
    char client_addr_str[INET_ADDRSTRLEN];
    inet_ntop(client_addr.ss_family, 
        get_in_addr((struct sockaddr *)&client_addr), 
        client_addr_str, 
        sizeof(client_addr_str));

    // Add this connection to the epoll instance, first make non-blocking
    if (-1 == make_non_blocking(client_fd)) {
      throw std::runtime_error("make_non_blocking");
    }

    add_to_read_poll(client_fd, std::string(client_addr_str));
    std::cout << "Connection accepted from " << client_addr_str << " on socket " 
      << client_fd << std::endl << std::flush;
  }

  if (-1 == client_fd && errno != EAGAIN && errno != EWOULDBLOCK) {
    throw std::runtime_error("accept");
  } 
}



void ConnectionPoller::init_listen_fd()
{
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof(hints)); 
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  std::string port_str = std::to_string(listen_port_);

  int rv = getaddrinfo(NULL, port_str.c_str(), &hints, &servinfo);
  if (0 != rv) { 
    throw std::runtime_error("getaddrinfo"); 
  }

  const int yes = 1;
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if (-1 == (listen_fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
      std::cout << "socket error";
      continue;
    }
    if (-1 == setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
      exit(1);
    }
    if (-1 == bind(listen_fd_, p->ai_addr, p->ai_addrlen)) {
      close(listen_fd_);
      std::cout << "listen error";
      continue;
    }
    break;
  }

  freeaddrinfo(servinfo);

  if (NULL == p) {
    throw std::runtime_error("bind"); 
  }

  if (-1 == make_non_blocking(listen_fd_)) {
    throw std::runtime_error("make_non_blocking");
  }

  if (-1 == listen(listen_fd_, 10)) {
    throw std::runtime_error("listen");
  }

  // Add listen_fd_ to epoll instance
  struct epoll_event event;
  auto connection_info = new Connection();
  connection_info->fd = listen_fd_;

  event.data.ptr = connection_info;
  event.events = EPOLLIN | EPOLLET;

  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event)) {
    throw std::runtime_error("epoll_ctl");
  }

  std::cout << "Listening on port " << listen_port_ << 
    ", socket " << listen_fd_ << std::endl << std::flush;
}

void ConnectionPoller::add_to_read_poll(int fd, std::string address)
{
  static struct epoll_event event;
  event.events = EPOLLIN | EPOLLET;

  if (-1 == make_non_blocking(fd)) {
    throw std::runtime_error("make_non_blocking");
  }

  auto connection_context = new Connection();
  connection_context->fd = fd;
  connection_context->address = address;
  event.data.ptr = connection_context;

  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)) {
    throw std::runtime_error("epoll_ctl");
  }
}

void* ConnectionPoller::get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


