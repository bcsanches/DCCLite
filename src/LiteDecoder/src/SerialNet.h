#ifndef _NET_CLIENT_H
#define _NET_CLIENT_H

namespace SerialNet
{
	void Init();

	void Send(const char *str);
	void SendLn(const char *str);
	void Send(int value);	

	void SendLog(const char *module, const char *msg, ...);	

	int Available();

	int ReadChar();
};

#endif
