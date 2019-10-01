#include <NetAPI/common.hpp>
#include <NetAPI/socket/Client.hpp>
#include <iostream>
int main() {
  NetAPI::Socket::Client cli;
  if (!cli.Connect("127.0.0.1", 7777)) {
    std::cerr << "Failed to connect to server" << std::endl;
  }
  while (cli.IsConnected()) {
  }
  return EXIT_SUCCESS;
}