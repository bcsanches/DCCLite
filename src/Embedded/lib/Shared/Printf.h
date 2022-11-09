// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <stdarg.h>

namespace dcclite
{
	class StringWrapper
	{
		public:
			inline StringWrapper(const char *str) noexcept :
				m_pszStr{ str }
			{
				//empty
			}

			inline const char ReadChar(unsigned pos) noexcept
			{
				return m_pszStr[pos];
			}

		private:
			const char *m_pszStr;
	};

	template <typename T, typename W>
	void PrintfImpl(T &stream, W format, va_list args)
	{
		unsigned int i = 0;		

		for (;;)
		{
			char ch = format.ReadChar(i++);

			switch (ch)
			{
				case '%':
				{
					ch = format.ReadChar(i++);
					switch (ch)
					{
						case 'c':
#ifdef ARDUINO
							ch = (char) va_arg(args, int);
#else
							ch = va_arg(args, char);
#endif
							stream.Print(ch);
							break;

						case 'd':
						{
							int n = va_arg(args, int);
							stream.Print(n);
							break;
						}

						case 'u':
						{
							unsigned n = va_arg(args, unsigned);
							stream.Print(n);
							break;
						}

						case 's':
						{
							const char *str = va_arg(args, const char *);
							stream.Print(str);
							break;
						}
					}
				}
				break;

				case '\\':
				{
					ch = format.ReadChar(i++);
					switch (ch)
					{
						case 'n':
#ifdef ARDUINO
							stream.Println();
#else
							stream.Print('\n');
#endif
							break;

						case 'r':
							stream.Print('\r');
							break;

						case 't':
							stream.Print('\t');
							break;

						case '\'':
							stream.Print('\'');
							break;

						case '\\':
							stream.Print('\\');
							break;

						case '\"':
							stream.Print('\"');
							break;
					}
				}
				break;

				case '\0':
					return;

				default:
					stream.Print(ch);
					break;
			}
		}	
	}

	template <typename T, typename W>
	void Printf(T &stream, W format, ...)
	{		
		va_list args;
		va_start(args, format);

		PrintfImpl(stream, format, args);

		va_end(args);
	}
}

