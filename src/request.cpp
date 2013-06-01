#include "request.h"
#include <algorithm>
#include <sstream>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

Request::Request() : raw_request(new buffer()), is_parsed(false), resource_size_(-1) {}

Request::~Request() { delete raw_request; }

bool Request::parse()
{
  request_type = STATIC; // temporary TODO

  request_body = find_string(raw_request->cbegin(), raw_request->cend(), std::string("\r\n\r\n"));
  if (request_body == raw_request->cend()) {
    return false;
  }

  buffer::const_iterator request_line_end = find_string(raw_request->cbegin(), request_body, std::string("\r\n")); // TODO error check 
  std::string request_line_str(raw_request->cbegin(), request_line_end);
  
  std::istringstream request_line_ss(request_line_str);
  std::copy(std::istream_iterator<std::string>(request_line_ss),
      std::istream_iterator<std::string>(),
      std::back_inserter<std::vector<std::string> >(request_line));

  parse_headers(request_line_end + 2, request_body + 2);
  
  return true;
}

Request::buffer::const_iterator Request::find_string(buffer::const_iterator begin, buffer::const_iterator end, std::string s) 
{
  return std::search(begin, end, s.begin(), s.end()); 
}

void Request::parse_headers(buffer::const_iterator begin, buffer::const_iterator end)
{
  buffer::const_iterator i = begin;
  buffer::const_iterator j;

  std::string test = std::string(begin, end);
  while (end != (j = find_string(i, end, std::string("\r\n")))) {
    auto separator = std::find(i, j, ':');
    std::pair<std::string, std::string> header;
    header.first = std::string(i, separator);
    header.second = std::string(separator + 2, j);
    headers.insert(header);
    i = j + 2;
  }
}

size_t Request::resource_size() {
  if (-1 == resource_size_) {
    struct stat info;
    int resource_fd;
    std::string file_name(request_line[1]);
    if (-1 == (resource_fd = open(file_name.c_str() + 1, O_RDONLY))) { // +1 for leading slash
      std::cout << strerror(errno) << std::endl;
    } 
    if (0 != fstat(resource_fd, &info)) { 
      perror("fstat() error"); 
    }
    resource_size_ = info.st_size;
  }
  return resource_size_;
}

bool Request::is_get() const {
  return request_line[0] == "GET";
}
