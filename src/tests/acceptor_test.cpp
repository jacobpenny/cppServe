#include "../acceptor.h"

#include "gtest/gtest.h"

TEST(AcceptorTest, ConstructorPort80) {
  Acceptor a("80");
  EXPECT_GT(a.get_listen_fd(), 0);
}

TEST(AcceptorTest, ConstructorPort3000) {
  Acceptor a("3000");
  EXPECT_GT(a.get_listen_fd(), 0);
}

