// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#ifndef CONSOLE_NET_H
#define CONSOLE_NET_H

#include "Strings.h"

namespace Console
{
	typedef __FlashStringHelper ConsoleFlashStringHelper_t;

	enum class Format
	{
		DECIMAL,
		HEXADECIMAL
	};

	extern void Send(const char *str);
	extern void Send(const ConsoleFlashStringHelper_t *fstr);
	extern void SendLn(const char *str);
	extern void Send(char value);		
	extern void Send(int value, Format format = Format::DECIMAL);
	extern void Send(unsigned int value, Format format = Format::DECIMAL);
	extern void Send(const unsigned long value, Format format = Format::DECIMAL);

	struct Hex
	{
		unsigned int num;

		explicit Hex(unsigned int n) :num{ n } {}
	};

	struct IpPrinter
	{
		const unsigned char *srcIp;

		explicit IpPrinter(const unsigned char src_ip[4]) : srcIp(src_ip) {}
	};

	namespace detail
	{	
		template <typename T>
		struct formatter
		{
			inline void format(const T &arg)
			{
				Send(arg);
			}
		};

		template <>
		struct formatter<char>
		{
			inline void format(char ch)
			{
				Send(ch);
			}
		};

		template <>
		struct formatter<const char *>
		{
			inline void format(const char *str)
			{
				Send(str);
			}
		};	

		template <>
		struct formatter<Hex>
		{
			inline void format(Hex hex)
			{
				Send(hex.num, Format::HEXADECIMAL);
			}
		};

		template <>
		struct formatter<IpPrinter>
		{
			inline void format(const IpPrinter &ip)
			{
				Send(ip.srcIp[0]);
				Send('.');
				Send(ip.srcIp[1]);
				Send('.');
				Send(ip.srcIp[2]);
				Send('.');
				Send(ip.srcIp[3]);
			}
		};

		template <>
		struct formatter<ConsoleFlashStringHelper_t>
		{			
			//static_assert(sizeof(ConsoleFlashStringHelper_t) == 1);

			inline void format(const ConsoleFlashStringHelper_t *fstr)
			{
				Send(fstr);				
			}
		};		

		template <typename T>
		inline void DoSendLog(const T &arg1)
		{
			formatter<T> f;
			f.format(arg1);
		}
		
		inline void DoSendLog(const ConsoleFlashStringHelper_t *arg1)
		{
			formatter<ConsoleFlashStringHelper_t> f;
			f.format(arg1);
		}

		template <typename T,typename... Args>
		inline void DoSendLog(const T &arg1, Args ...args)
		{
			DoSendLog(arg1);

			DoSendLog(args...);
		}
	}	

	extern void Init();	

	template <typename... Args>
	inline void SendLogEx(const char *module, Args... args)
	{		
		Console::Send('[');
		Console::Send(module);
		Console::Send(']');

		Console::Send(' ');

		detail::DoSendLog(args...);

		Console::SendLn(" ");
	}

	template <typename... Args>
	inline void SendLogEx(const ConsoleFlashStringHelper_t *fmodule, Args... args)
	{
		Console::Send('[');

		detail::formatter<ConsoleFlashStringHelper_t> ffstr;
		ffstr.format(fmodule);

		Console::Send(']');
		Console::Send(' ');

		detail::DoSendLog(args...);

		Console::SendLn(" ");
	}	

	extern int Available();

	extern int ReadChar();

	extern void Update();

	extern bool Custom_ParseCommand(const char *command);
};

#endif
