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
	this->requestedItemId = requestedItemId;
	this->contributedItemId = contributedItemId;
	this->uccLocation = uccLocation;
	this->searchDepth = searchDepth;
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
		PacketRequestConstraint packetbody;
		packetbody.Read(stream);
		if (packetbody._constraintItemId == this->contributedItemId) {
			agreement = true;
	
			setState(ST_SEND_CONSTRAINT);

			ConstraintResolve(true);
		}
		else {
			if (searchDepth >= MAX_SEARCH_DEPTH) {
				agreement = false;

				setState(ST_SEND_CONSTRAINT);

				ConstraintResolve(true);
			}
			else {
				//we create a child mcp
				if (_mcp != nullptr)
					destroyChildMCP();
				_mcp = App->agentContainer->createMCP(node(), packetbody._constraintItemId, contributedItemId, searchDepth);
				setState(ST_FINISH_CONSTRAINT);
			}
		}
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

bool UCP::ConstraintResolve(bool accepted)
{
	PacketHeader packethead;
	packethead.packetType = PacketType::ResultConstraint;
	packethead.dstAgentId = uccLocation.agentId;
	packethead.srcAgentId = this->id();

	PacketResultConstraint body;
	body.accepted = accepted;
	OutputMemoryStream stream;
	packethead.Write(stream);
	body.Write(stream);

	return sendPacketToAgent(uccLocation.hostIP, uccLocation.hostPort, stream);
}
