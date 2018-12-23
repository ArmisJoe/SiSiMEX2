#include "MCP.h"
#include "UCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"

// MCP
enum State
{
	ST_INIT,
	ST_REQUESTING_MCCs,
	ST_ITERATING_OVER_MCCs,

	// TODO: Other states
	ST_WAITING_ACCEPTANCE,
	ST_NEGOTIATING,
	ST_WAIT_RESULT,
	ST_NEGOTIATION_FINISHED
};

MCP::MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth) :
	Agent(node),
	_requestedItemId(requestedItemID),
	_contributedItemId(contributedItemID),
	_searchDepth(searchDepth)
{
	setState(ST_INIT);
}

MCP::~MCP()
{
}

void MCP::update()
{
	switch (state())
	{
	case ST_INIT:
		queryMCCsForItem(_requestedItemId);
		setState(ST_REQUESTING_MCCs);
		break;

	case ST_REQUESTING_MCCs:
		break;

	case ST_ITERATING_OVER_MCCs:
		// TODO: Handle this state
		if (_mccRegisterIndex < _mccRegisters.size()) {
			AskNegotiation(_mccRegisters[_mccRegisterIndex]);
			setState(ST_WAITING_ACCEPTANCE);
		}
		else {
			setState(ST_NEGOTIATION_FINISHED);
			_mccRegisterIndex = 0;
		}
		break;

	// TODO: Handle other states
	case ST_WAITING_ACCEPTANCE:
		break;

	case ST_NEGOTIATING:
		if (_ucp != nullptr && _ucp->state() == ST_NEGOTIATION_FINISHED) 
		{
			if (_ucp->agreement == false) 
			{								 
				setState(ST_ITERATING_OVER_MCCs);
				_mccRegisterIndex++;
		
			}
			else
			{
				setState(ST_NEGOTIATION_FINISHED);
			}
		}
		break;

	case ST_WAIT_RESULT:
		break;

	case ST_NEGOTIATION_FINISHED:
		destroyChildUCP();
		break;
	default:
		break;
	}
}

void MCP::stop()
{
	// TODO: Destroy the underlying search hierarchy (UCP->MCP->UCP->...)
	destroyChildUCP();
	destroy();
}

void MCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ReturnMCCsForItem:
		if (state() == ST_REQUESTING_MCCs)
		{
			// Read the packet
			PacketReturnMCCsForItem packetData;
			packetData.Read(stream);

			// Log the returned MCCs
			for (auto &mccdata : packetData.mccAddresses)
			{
				uint16_t agentId = mccdata.agentId;
				const std::string &hostIp = mccdata.hostIP;
				uint16_t hostPort = mccdata.hostPort;
			}

			// Store the returned MCCs from YP
			_mccRegisters.swap(packetData.mccAddresses);

			// Select the first MCC to negociate
			_mccRegisterIndex = 0;
			setState(ST_ITERATING_OVER_MCCs);

			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::ReturnMCCsForItem was unexpected.";
		}
		break;

	// TODO: Handle other packets
	case PacketType::ResponseNegotiation:
		if (state() == ST_WAITING_ACCEPTANCE)
		{
			PacketResponseNegotiation packetBody;
			packetBody.Read(stream);
			if (packetBody.acceptNegotiation == true) {
				createChildUCP(packetBody.uccLoc);
				setState(ST_NEGOTIATING);
			}
			else {
				setState(ST_ITERATING_OVER_MCCs);
				_mccRegisterIndex++;
			}
		}
		break;
	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCP::negotiationFinished() const
{
	return state() == ST_NEGOTIATION_FINISHED;
}

bool MCP::negotiationAgreement() const
{
	// TODO: Did the child UCP find a solution?
	return false; 
	if (_ucp != nullptr) {
		return _ucp->agreement == true; 
	}
	else {
		return false;
	}
}


bool MCP::AskNegotiation(AgentLocation & mcc)
{
	PacketHeader packetHeader;
	packetHeader.packetType = PacketType::RequestNegotiation;
	packetHeader.dstAgentId = mcc.agentId;
	packetHeader.srcAgentId = this->id();

	OutputMemoryStream stream;
	packetHeader.Write(stream);

	return sendPacketToAgent(mcc.hostIP, mcc.hostPort, stream);
}

bool MCP::queryMCCsForItem(int itemId)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::QueryMCCsForItem;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketQueryMCCsForItem packetData;
	packetData.itemId = _requestedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	// 1) Ask YP for MCC hosting the item 'itemId'
	return sendPacketToYellowPages(stream);
}

void MCP::destroyChildUCP()
{
	if (_ucp != nullptr) {
		_ucp->stop();
		_ucp.reset();
	}
}

void MCP::createChildUCP(AgentLocation & uccLoc)
{
	if (_ucp != nullptr)
		destroyChildUCP();
	_ucp = App->agentContainer->createUCP(node(), requestedItemId(), contributedItemId(), uccLoc, searchDepth() + 1);
}
