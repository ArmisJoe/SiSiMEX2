#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


// TODO: Make an enum with the states
enum State {
	ST_INIT,
	ST_ITEM_REQUESTED,
	ST_FINISH_CONSTRAINT,
	ST_SEND_CONSTRAINT,
	ST_NEGOTIATION_FINISHED

};

UCP::UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLocation, unsigned int searchDepth) :
	Agent(node)
{
	// TODO: Save input parameters
}

UCP::~UCP()
{
}

void UCP::update()
{
	switch (state())
	{
		// TODO: Handle states
	case ST_INIT:
		agreement = -1;
		// we will be requesting right away
		break;

	case ST_ITEM_REQUESTED:
		break;

	case ST_FINISH_CONSTRAINT:
		break;

	case ST_SEND_CONSTRAINT:
		break;

	case ST_NEGOTIATION_FINISHED:
		break;

	default:;
	}
}

void UCP::stop()
{
	// TODO: Destroy search hierarchy below this agent
	destroyChildMCP();
	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::RequestConstraint:
		break;

	case PacketType::AckConstraint:
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCP::destroyChildMCP()
{
	if (_mcp != nullptr) {
		_mcp->stop();
		_mcp.reset();
	}
}