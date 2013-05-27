#include "request.h"
#include <algorithm>
#include <sstream>
#include <iterator>


Request::Request() : raw_request(new buffer(1024)), is_parsed(false) {}

Request::~Request() { delete raw_request; }

bool Request::parse()
{
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

