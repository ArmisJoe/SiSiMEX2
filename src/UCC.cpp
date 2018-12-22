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

		if (state() == ST_ITEM_REQUEST)
		{
			PacketRequestItem packetBody;
			packetBody.Read(stream);
			PacketHeader _packetHeader;
			_packetHeader.srcAgentId = id();
			_packetHeader.dstAgentId = packetHeader.srcAgentId;
			_packetHeader.packetType = PacketType::RequestConstraint;
			OutputMemoryStream _stream;
			_packetHeader.Write(_stream);
			PacketRequestConstraint _packetBody;
			_packetBody._constraintItemId = constraintItemId;
			_packetBody.Write(_stream);
			socket->SendPacket(_stream.GetBufferPtr(), _stream.GetSize());
			setState(ST_ITEM_CONSTRAINT);
		}
		else 
		{
			wLog << "UCC::PacketReceived() - Unexpected Item Request";
		}
		break;
	case PacketType::ResultConstraint:
		if (state() == ST_ITEM_CONSTRAINT)
		{
			PacketResultConstraint packetBody;
			packetBody.Read(stream);
			if (packetBody.accepted == true) {
				agreement = true;
			}
			else {
				agreement = false;
			}
			PacketHeader _packetHeader;
			_packetHeader.srcAgentId = id();
			_packetHeader.dstAgentId = packetHeader.srcAgentId;
			_packetHeader.packetType = PacketType::AckConstraint;
			OutputMemoryStream _stream;
			_packetHeader.Write(_stream);
			socket->SendPacket(_stream.GetBufferPtr(), _stream.GetSize());
			setState(ST_FINISH_NEGOTIATION);
		}
		else
		{
			wLog << "UCC::PacketReceived() - Unexpected Item Request";
		}
		break;

	default:
		wLog << "UCC: OnPacketReceived() - Unexpected PacketType.";
	}
}
