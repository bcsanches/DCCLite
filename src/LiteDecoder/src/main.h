#pragma once

#ifdef LITEDECODERLIB_EXPORTS  
#define LITEDECODER_API __declspec(dllexport)   
#else  
#define LITEDECODER_API __declspec(dllimport)   
#endif


extern "C" LITEDECODER_API void setup();
extern "C" LITEDECODER_API void loop();



