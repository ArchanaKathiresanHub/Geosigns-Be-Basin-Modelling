#include<iostream>
void hello()
{
  std::cout <<
    "Hello world, I'm a thread!"
    << std::endl;
}

int main(int argc, char* argv[])
{
  boost::thread thrd(&hello);
  thrd.join();
  return 0;
}

///usr/bin/g++ -lboost_thread thread.cpp
