#pragma once

#include <map>
#include <string>
#include <memory>

class Object
{
	public:
		Object(std::string name);

		void AddChild(std::unique_ptr<Object> obj);

	private:
		std::string m_strName;

		std::map<std::string_view, std::unique_ptr<Object>> m_mapObjects;
};
