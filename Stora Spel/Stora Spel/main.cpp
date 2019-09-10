#include <iostream>

#include <pistol/graphics.h>
#include "testphysics.h"
#include "testSound.h"
#include "testNetwork.h"

//#include <glad/glad.h>


int main(unsigned argc, char **argv)
{
	std::cout << "Hello World!*!!!111\n";

	std::cout << "Test från development " << testPhysics() << " " << testSound() << " " << testNetwork();

	std::cout << "Test från development2 " << GraphicsTestPistol() << "\n";

	return EXIT_SUCCESS;
}