#pragma once

#include <stdint.h>

class EpromStream;

namespace Session
{
	extern void LoadConfig(EpromStream &stream);
	extern void SaveConfig(EpromStream &stream);

	extern bool Init();

	extern bool Configure(const uint8_t *srvIp, uint16_t srvport);

	extern void Update();

	extern void LogStatus();
}
