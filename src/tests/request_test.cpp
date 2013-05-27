#include "../request.h"

#include <vector>
#include <fstream> 
#include "gtest/gtest.h"

TEST(RequestTest, Parsing) {
  std::ifstream request_file("sample_request", std::ios::binary);
  Request::buffer *raw_request = new Request::buffer(std::istreambuf_iterator<char>(request_file), 
                                std::istreambuf_iterator<char>());
  Request request;
  request.raw_request = raw_request;
  EXPECT_EQ(true, request.parse());
  EXPECT_EQ(request.request_line[0], "GET");
  EXPECT_EQ(request.request_line[1], "/");
  EXPECT_EQ(request.request_line[2], "HTTP/1.1");
  EXPECT_EQ(request.headers["Connection"], "Keep-Alive");

}

