#include <iostream>

#include <entt.hpp>
#include "PrintPositionSystem.h"
#include <glob/graphics.h>
#include "testphysics.h"
#include "testSound.h"
#include "testNetwork.h"

//#include <glad/glad.h>



int main(unsigned argc, char **argv) {
	std::cout << "Hello World!*!!!111\n";

	std::cout << "Test fr�n development\n";

	 entt::registry registry;

     auto entity = registry.create();
     registry.assign<Position>(entity, 1.0f, 2.0f, 3.0f);
     registry.assign<Velocity>(entity, 3.0f, 2.0f, 1.0f);
     
     print(registry);
	std::cout << "Test fr�n development " << testPhysics() << " " << testSound() << " " << testNetwork();

	std::cout << "Test fr�n development2 " << glob::GraphicsTest() << "\n";

	return EXIT_SUCCESS;
}