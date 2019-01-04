#pragma once

#ifdef WIN32
	#ifdef LITEDECODERLIB_EXPORTS  
	#define LITEDECODER_API __declspec(dllexport)   
	#else  
	#define LITEDECODER_API __declspec(dllimport)   
	#endif
#else
	#define LITEDECODER_API
#endif


extern "C" LITEDECODER_API void setup();
extern "C" LITEDECODER_API void loop();



