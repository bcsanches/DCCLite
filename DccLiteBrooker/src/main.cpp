#include <iostream>
#include <stdexcept>

#include <boost/log/trivial.hpp>

#include "Brooker.h"

#include "LogUtils.h"

int main(int argc, char **argv)
{
	dcclite::InitLog("DccLiteBrooker_%N.log");

	const char *configFileName = (argc == 1) ? "config.json" : argv[1];	

	try
	{ 
		Brooker brooker;

		brooker.LoadConfig(configFileName);
		
		brooker.Update();
	}	
	catch (std::exception &ex)
	{
		BOOST_LOG_TRIVIAL(fatal) << "caught " << ex.what();	
	}

	return 0;
}
