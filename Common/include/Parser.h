#pragma once

namespace dcclite
{
	enum Tokens
	{
		TOKEN_ID,
		TOKEN_NUMBER,
		TOKEN_HEX_NUMBER,

		TOKEN_CMD_START,
		TOKEN_CMD_END,

		TOKEN_EOF,

		TOKEN_ERROR
	};

	class Parser
	{
		private:
			const char *m_pszCmd;

			unsigned int m_iPos;
			unsigned int m_iLastKnowPos;

		public:
			Parser(const char *cmd);

			Tokens GetToken(char *dest, unsigned int destSize);
			Tokens GetNumber(int &dest);

			void PushToken();
	};
}

