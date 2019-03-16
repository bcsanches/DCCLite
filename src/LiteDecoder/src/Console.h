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

#include <avr/pgmspace.h>

namespace Console
{
	enum class Format
	{
		DECIMAL,
		HEXADECIMAL
	};

	extern void Send(const char *str);
	extern void SendLn(const char *str);
	extern void Send(char value);		
	extern void Send(int value, Format format = Format::DECIMAL);
	extern void Send(unsigned int value, Format format = Format::DECIMAL);

	struct Hex
	{
		unsigned int num;

		Hex(unsigned int n) :num{ n } {}
	};

	struct IpPrinter
	{
		const unsigned char *srcIp;

		IpPrinter(const unsigned char src_ip[4]) : srcIp(src_ip) {}
	};

	struct FlashStr
	{
		const char *fstr;

		explicit FlashStr(const char *p): fstr{ p } {}
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
		struct formatter<FlashStr>
		{
			inline void format(FlashStr fstr)
			{
				// Send(hex.num, Format::HEXADECIMAL);
				auto len = strlen_P(fstr.fstr);
				for (size_t i = 0; i < len; ++i)
				{
					Send(static_cast<char>(pgm_read_byte_near(fstr.fstr + i)));
				}
			}
		};

		template <typename T>
		inline void DoSendLog(const T &arg1)
		{
			formatter<T> f;
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
		Console::Send("<LOG");
		Console::Send(' ');
		Console::Send(module);

		Console::Send(' ');

		detail::DoSendLog(args...);

		Console::SendLn(">");
	}

	template <typename... Args>
	inline void SendLogEx(const FlashStr &module, Args... args)
	{
		Console::Send("<LOG");
		Console::Send(' ');

		detail::formatter<FlashStr> ffstr;
		ffstr.format(module);		

		Console::Send(' ');

		detail::DoSendLog(args...);

		Console::SendLn(">");
	}	

	extern int Available();

	extern int ReadChar();

	extern void Update();
};

#endif
