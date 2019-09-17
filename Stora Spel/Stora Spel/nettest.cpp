#include <NetAPI/networkTest.hpp>
#include <NetAPI/socket/server.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <iostream>
int main() {
  NetAPI::Socket::Server server;
  std::cout << "Size of server: " << sizeof(server) << std::endl;
  const char* msg = "Huehue\n\0";
  NetAPI::Socket::Data cool_data;
  cool_data.buffer = msg;
  cool_data.ID = NetAPI::Socket::EVERYONE;
  cool_data.len = strlen(msg);
  if (!server.Setup(1234)) {
    std::cout << "Failed to setup server\n";
  }
  bool sent = false;
  while (true) {
    if (!server.Update()) {
      std::cout << "Failed to update \n";
      break;
    }
    for (unsigned i = 0; i < server.GetConnectedPlayers(); i++) {
      NetAPI::Socket::Data d = server[i];
      if (server.HasData(d) && !server.SocketDisconnected(d)) {
        std::cout << d.buffer << std::endl;
        if (!sent) {
          server.SendToAll(cool_data);
          sent = true;
        }
      }
    }
  }
  server.Cleanup();
}