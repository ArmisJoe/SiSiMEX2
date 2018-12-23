#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


// TODO: Make an enum with the states
enum State 
{
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
	this->Loc = uccLocation;
	this->searchDepth = searchDepth;
}

UCP::~UCP()
{
}

bool UCP::negotiationFinished()
{
	return state() == ST_NEGOTIATION_FINISHED;
}


void UCP::update()
{
	OutputMemoryStream stream;
	PacketHeader packetHeader;

	PacketRequestItem body;
	switch (state())
	{
		// TODO: Handle states
	case ST_INIT:
		setState(ST_ITEM_REQUESTED);
		agreement = -1;
		// we will be requesting right away
	
		packetHeader.packetType = PacketType::RequestItem;
		packetHeader.dstAgentId = Loc.agentId;
		packetHeader.srcAgentId = id();

		body._requestedItemId = requestedItemId;
		
		packetHeader.Write(stream);
		body.Write(stream);

		sendPacketToAgent(Loc.hostIP, Loc.hostPort, stream);
		
		break;

	case ST_FINISH_CONSTRAINT:
		if (mcp->negotiationFinished()) 
		{
			if (mcp->negotiationAgreement()) 
			{
				ConstraintResolve(true);
				agreement = true;
				
			}
			else 
			{
				ConstraintResolve(false);
				agreement = false;
			}
			setState(ST_SEND_CONSTRAINT);
		}
		break;

	default:
		break;
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
		PacketRequestConstraint packetReqCon;
		packetReqCon.Read(stream);
		if (packetReqCon._constraintItemId == this->contributedItemId)
		{
			agreement = true;
	
			setState(ST_SEND_CONSTRAINT);

			ConstraintResolve(true);
		}
		else {
			if (searchDepth >= MAX_SEARCH_DEPTH) 
			{
				agreement = false;

				setState(ST_SEND_CONSTRAINT);

				ConstraintResolve(true);
			}
			else 
			{	
				setState(ST_FINISH_CONSTRAINT);
				//we create a child mcp
				if (mcp != nullptr)
					destroyChildMCP();
				mcp = App->agentContainer->createMCP(node(), packetReqCon._constraintItemId, contributedItemId, searchDepth);
			
			}
		}
		break;

	case PacketType::AckConstraint:
		if (state() == ST_SEND_CONSTRAINT) 
		{
			setState(ST_NEGOTIATION_FINISHED);
		}
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCP::destroyChildMCP()
{
	if (mcp == nullptr)
		return;
		mcp->stop();
		mcp.reset();
	
}

bool UCP::ConstraintResolve(bool accepted)
{
	PacketHeader packetHeader;
	packetHeader.packetType = PacketType::ResultConstraint;
	packetHeader.dstAgentId = Loc.agentId;
	packetHeader.srcAgentId = this->id();

	PacketResultConstraint packetResCon;
	packetResCon.accepted = accepted;
	OutputMemoryStream stream;
	packetHeader.Write(stream);
	packetResCon.Write(stream);
	return sendPacketToAgent(Loc.hostIP, Loc.hostPort, stream);
}
