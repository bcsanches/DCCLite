#pragma once

#include "Service.h"

#include <map>
#include <string>

#include "DecoderManager.h"

class Node;

class DccLiteService : public Service
{
	public:
		DccLiteService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params);

		virtual ~DccLiteService();

	private:
		std::map<std::string, std::unique_ptr<Node>> m_mapNodes;

		DecoderManager m_clDecoderManager;
};
