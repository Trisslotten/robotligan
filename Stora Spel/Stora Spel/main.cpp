#include <iostream>

#include "Test.h"
#include "testphysics.h"
#include "testSound.h"

int main(unsigned argc, char **argv)
{
	std::cout << "Hello World!*!!!111\n";

	std::cout << "Test från development" << testGraphics() << " " << testPhysics() << " " << testSound();

	return EXIT_SUCCESS;
}