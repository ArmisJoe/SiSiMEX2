#include "MCC.h"
#include "UCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"

// ORDER
//packetHead.packetType
//packetHead.srcAgentId 
//packetHead.dstAgentId 

enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	
	// TODO: Other states
	ST_NEGOTIATING,
	ST_FINISHED,

};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	setState(ST_INIT);
}


MCC::~MCC()
{
}

void MCC::update()
{
	switch (state())
	{
	case ST_INIT:
		if (registerIntoYellowPages()) 
		{
			setState(ST_REGISTERING);
		}
		else 
		{
			setState(ST_FINISHED);
		}
		break;

	case ST_REGISTERING:
		// See OnPacketReceived()
		break;

		// TODO: Handle other states
	case ST_NEGOTIATING:
		if (_ucc != nullptr && _ucc->negotiationfinished() == true) 
		{
			if (negotiationAgreement()) 
			{
				setState(ST_FINISHED);																
			}
			destroyChildUCC();
		}
		break;

	case ST_FINISHED:
		destroyChildUCC();
		destroy();
	}
}

void MCC::stop()
{
	// Destroy hierarchy below this agent (only a UCC, actually)
	destroyChildUCC();

	unregisterFromYellowPages();
	setState(ST_FINISHED);
	
	destroy();
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::RegisterMCCAck:
		if (state() == ST_REGISTERING)
		{
			setState(ST_IDLE);
			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RegisterMCCAck was unexpected.";
		}
		break;

	case PacketType::RequestNegotiation:
		if (state() == ST_IDLE)
		{
			AgentLocation Loc;
			createChildUCC();
			Loc.agentId = _ucc->id();
			Loc.hostIP = socket->RemoteAddress().GetIPString();
			Loc.hostPort = LISTEN_PORT_AGENTS;
			sendAcceptNegotiation(socket, packetHeader.srcAgentId, true, Loc);
			setState(ST_NEGOTIATING);
		}
		else
		{
			AgentLocation Loc;
			sendAcceptNegotiation(socket, packetHeader.srcAgentId, false, Loc);
			wLog << "OnPacketReceived() - PacketType::RequestNegotiation was unexpected.";
		}
		break;
	

	// TODO: Handle other packets

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCC::isIdling() const
{
	return state() == ST_IDLE;
}

bool MCC::negotiationFinished() const
{
	return state() == ST_FINISHED;
}

bool MCC::negotiationAgreement() const
{
	// If this agent finished, means that it was an agreement
	// Otherwise, it would return to state ST_IDLE
	return _ucc->negotiationagreement() == true;
}

bool MCC::sendAcceptNegotiation(TCPSocketPtr socket, uint16_t dstID, bool accept, AgentLocation &uccLoc)
{
	PacketHeader packetHeader;
	packetHeader.packetType = PacketType::ResponseNegotiation;
	packetHeader.srcAgentId = id();
	packetHeader.dstAgentId = dstID;
	PacketResponseNegotiation packetResponse;
	packetResponse.acceptNegotiation = accept;
	packetResponse.uccLoc = uccLoc;

	OutputMemoryStream stream;
	packetHeader.Write(stream);
	packetResponse.Write(stream);

	socket->SendPacket(stream.GetBufferPtr(), stream.GetSize());

	return false;
}

bool MCC::registerIntoYellowPages()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RegisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketRegisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	return sendPacketToYellowPages(stream);
}

void MCC::unregisterFromYellowPages()
{
	// Create message
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UnregisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketUnregisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	sendPacketToYellowPages(stream);
}

void MCC::createChildUCC()
{
	// TODO: Create a unicast contributor

	if (_ucc != nullptr)
		destroyChildUCC();
	_ucc = App->agentContainer->createUCC(node(), contributedItemId(), constraintItemId());
}

void MCC::destroyChildUCC()
{
	// TODO: Destroy the unicast contributor child

	if (_ucc == nullptr)
		return;
	
		_ucc->stop();
		_ucc.reset();
	
}
