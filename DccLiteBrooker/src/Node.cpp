#include "Node.h"

Node::Node(const std::string &name, DccLiteService &dccService, const nlohmann::json &params) :
	m_strName(name),
	m_clDccService(dccService)
{
	//empty
}