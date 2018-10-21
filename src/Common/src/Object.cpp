#include "Object.h"

#include <sstream>

namespace dcclite
{
	Object::Object(std::string name) :
		GenericObject<Object>(std::move(name))
	{
		//empty
	}
}
