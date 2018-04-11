#ifndef BLINKER_H
#define BLINKER_H

namespace Blinker
{
	enum class Animations
	{
		OK,
		ERROR
	};

	extern void Init();

	extern void Update();

	extern void Play(Animations animation);
}

#endif