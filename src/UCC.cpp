#include "UCC.h"


// TODO: Make an enum with the states
enum State
{
	ST_ITEM_REQUEST,
	ST_ITEM_CONSTRAINT,
	ST_FINISH_NEGOTIATION
};


UCC::UCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node)
{
	// TODO: Save input parameters
}

UCC::~UCC()
{
}

void UCC::stop()
{
	destroy();
}

void UCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::RequestItem:
		break;
	case PacketType::ResultConstraint:
		break;

	default:
		wLog << "UCC: OnPacketReceived() - Unexpected PacketType.";
	}
}
