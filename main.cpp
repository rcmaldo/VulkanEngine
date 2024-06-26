#include "first_app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	vulkanengine::FirstApp app{};

	try
	{
		app.Run();

	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}