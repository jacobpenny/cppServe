#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <vector>
#include <map>
#include <string>

class Request {
  public:
    enum RequestType { STATIC, CGI };
    enum RequestMethod { GET, HEAD };
    typedef std::vector<char> buffer;

  public:
    Request();
    ~Request();
    bool parse();
  
  public:
    buffer *raw_request;
    buffer::const_iterator request_body;

    RequestType request_type;
    
    RequestMethod request_method;
    std::vector<std::string> request_line;
    std::map<std::string, std::string> headers;

    bool is_parsed;
    bool has_body;
    int write_marker;

  private:
    // buffer::iterator prev_position_;
    buffer::const_iterator find_string(buffer::const_iterator begin, buffer::const_iterator end, std::string s);
    void parse_headers(buffer::const_iterator begin, buffer::const_iterator end);
};

#endif
