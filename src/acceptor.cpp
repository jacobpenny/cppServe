#include "acceptor.h"
#include "connection.h"
#include "utils.h"

#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h> // memset
#include <arpa/inet.h> // inet_ntop
#include <netinet/tcp.h> // TCP_NODELAY

Acceptor::Acceptor(const char *port)
{
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof(hints)); 
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int rv = getaddrinfo(NULL, port, &hints, &servinfo);
  if (0 != rv) { 
    throw std::runtime_error("getaddrinfo"); 
  }

  const int yes = 1;
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if (-1 == (listen_fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
      continue;
    }
    if (-1 == setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
      exit(1);
    }
    if (-1 == bind(listen_fd_, p->ai_addr, p->ai_addrlen)) {
      close(listen_fd_);
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
}

Acceptor::~Acceptor() 
{
  close(listen_fd_); 
}

std::vector<Connection*> Acceptor::accept_connections() const
{
  static struct sockaddr_storage client_addr;
  static socklen_t sin_size;
  int client_fd;
  std::vector<Connection*> connections;

  sin_size = sizeof(client_addr);

  while (-1 != (client_fd = accept(listen_fd_, (struct sockaddr *)&client_addr, &sin_size))) {
    char client_addr_str[INET_ADDRSTRLEN];
    inet_ntop(client_addr.ss_family, 
        get_in_addr((struct sockaddr *)&client_addr), 
        client_addr_str, 
        sizeof(client_addr_str));

    if (-1 == make_non_blocking(client_fd)) {
      throw std::runtime_error("make_non_blocking");
    }

    std::cout << "Connection accepted from " << client_addr_str << " on socket " 
      << client_fd << std::endl << std::flush;

    connections.push_back(new Connection(client_fd, std::string(client_addr_str)));
  }

  if (-1 == client_fd && errno != EAGAIN && errno != EWOULDBLOCK) {
    throw std::runtime_error("accept");
  } 

  return connections;
}

void* Acceptor::get_in_addr(struct sockaddr *sa) 
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
