#include "../poller.h"
#include "../acceptor.h"

#include "gtest/gtest.h"

TEST(PollerTest, Constructor) {
  Acceptor a("3000");
  Poller p(a);
}

