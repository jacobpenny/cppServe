#!/bin/bash

g++ -std=c++0x utils.cpp connection.cpp acceptor.cpp poller.cpp main.cpp -pthread -o cppServe -g -Wall -Wextra -pedantic
