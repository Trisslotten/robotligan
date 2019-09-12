#include <iostream>

#include <glob/graphics.h>
#include <entt.hpp>
#include <NetAPI/networkTest.h>
#include <NetAPI/socket/client.h>
#include <NetAPI/socket/tcplistener.h>
#include "PrintPositionSystem.h"
#include "util/meminfo.h"
#include <thread>
#include <chrono>
//#include <glad/glad.h>

int main(unsigned argc, char **argv) {

  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";
  entt::registry registry;

  auto entity = registry.create();
  registry.assign<Position>(entity, 1.0f, 2.0f, 3.0f);
  registry.assign<Velocity>(entity, 3.0f, 2.0f, 1.0f);

  print(registry);

  std::cout << "Test från development2 " << glob::GraphicsTest() << "\n";

  std::cout << "WSA is initialized? " << std::boolalpha << NetAPI::initialization::WinsockInitialized() << std::endl;

  NetAPI::Socket::tcplistener server;
  NetAPI::Socket::tcpclient sclient;
  std::cout << "Sizeof client: " << sizeof(sclient) << std::endl;
  std::cout << "Sizeof server: " << sizeof(sclient) << std::endl;
  if (!server.Bind(1234))
  {
	  std::cout << "Failed to bind server" << std::endl;
  }
  std::cout << "Listening for clients" << std::endl;
  while (!sclient.isconnected())
  {
	  sclient = server.Accept();
  }
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM() << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM() << " MB\n";
  std::cout << "Client connected!" << std::endl;
  const char* buffer = {};
  do 
  {
	  buffer = server.Recv(sclient.GetLowLevelSocket());
	  if (strcmp(buffer, NetAPI::Common::no_data_available))
	  {
		  std::cout << "Client sent: ";
		  std::cout << buffer << std::endl;
		  sclient.disconnect();
		  break;
	  }
  } while (buffer != nullptr);
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM() << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM() << " MB\n";
  std::cin.ignore();
  return EXIT_SUCCESS;
}