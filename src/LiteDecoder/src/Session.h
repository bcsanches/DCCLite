#pragma once

#include <stdint.h>

class EpromStream;

namespace dcclite
{
	class Guid;
}

namespace Session
{
	extern void LoadConfig(EpromStream &stream);
	extern void SaveConfig(EpromStream &stream);

	extern bool Init();

	extern bool Configure(const uint8_t *srvIp, uint16_t srvport);

	extern void Update();

	extern void LogStatus();

	extern const dcclite::Guid &GetConfigToken();

	//This is to be only called by DecoderManager when loading its data
	extern void ReplaceConfigToken(const dcclite::Guid &configToken);
}
