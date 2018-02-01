#pragma once

#ifdef _WIN64
#define DCCLITE64
#elif defined _WIN32
#define DCCLITE32
#endif

#if (!defined DCCLITE64) && (!defined DCCLITE32)
#error "plataform not defined"
#endif
