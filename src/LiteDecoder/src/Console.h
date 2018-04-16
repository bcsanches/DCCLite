#ifndef CONSOLE_NET_H
#define CONSOLE_NET_H

namespace Console
{
	extern void Init();

	extern void Send(const char *str);
	extern void SendLn(const char *str);
	extern void Send(int value);	

	extern void SendLog(const char *module, const char *msg, ...);	

	extern int Available();

	extern int ReadChar();
};

#endif
