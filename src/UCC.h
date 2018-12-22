#pragma once
#include "Agent.h"

class UCC :
	public Agent
{
public:

	// Constructor and destructor
	UCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId);
	~UCC();

	// Agent methods
	void update() override { }
	void stop() override;
	UCC* asUCC() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	bool negotiationfinished() { return state() == ST_FINISH_NEGOTIATION; }
	bool negotiationagreement() { return (state() == ST_FINISH_NEGOTIATION && agreement == true); }

	// TODO
	uint16_t constraintItemId;
	uint16_t contributedItemId;
	int agreement = -1;
};

