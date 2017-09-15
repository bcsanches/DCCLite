#include <iostream>
#include <stdexcept>

#include "Brooker.h"

int main(int argc, char **argv)
{
	const char *configFileName = (argc == 1) ? "config.json" : argv[1];	

	try
	{ 
		Brooker brooker;

		brooker.LoadConfig(configFileName);
	}	
	catch (std::exception &ex)
	{
		std::cerr << "caught: " << ex.what();
	}

	return 0;
}
