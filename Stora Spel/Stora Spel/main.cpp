#include <iostream>

#include "Test.h"
#include "testphysics.h"
#include "testSound.h"
#include "testNetwork.h"

//#include <glad/glad.h>


int main(unsigned argc, char **argv)
{
	std::cout << "Hello World!*!!!111\n";

	std::cout << "Test från development" << testGraphics() << " " << testPhysics() << " " << testSound() << " " << testNetwork();

	return EXIT_SUCCESS;
}