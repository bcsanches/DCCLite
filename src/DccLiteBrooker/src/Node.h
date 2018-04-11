#pragma once

#include <string>

#include "json.hpp"

class DccLiteService;
class Decoder;

class Node
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE
		};

	public:
		Node(const std::string &name, DccLiteService &dccService, const nlohmann::json &params);

		Node(const Node &) = delete;
		Node(Node &&) = delete;

	private:
		std::string m_strName;

		DccLiteService &m_clDccService;

		std::vector<Decoder *> m_vecDecoders;

		Status					m_eStatus;
};
