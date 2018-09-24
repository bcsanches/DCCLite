#pragma once

#ifdef ARDUINOLIB_EXPORTS  
#define ARDUINO_API __declspec(dllexport)   
#else  
#define ARDUINO_API __declspec(dllimport)   
#endif
