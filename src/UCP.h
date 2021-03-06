#pragma once
#include "Agent.h"

// Forward declaration
class MCP;
using MCPPtr = std::shared_ptr<MCP>;

class UCP :
	public Agent
{
public:

	// Constructor and destructor
	UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLoc, unsigned int searchDepth);
	~UCP();

	bool negotiationFinished();

	// Agent methods
	void update() override;
	void stop() override;
	UCP* asUCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	void destroyChildMCP();

	bool ConstraintResolve(bool outcome);

	// TODO
	int agreement = false;
	uint16_t requestedItemId;
	uint16_t contributedItemId;
	AgentLocation Loc;
	unsigned int searchDepth;
	MCPPtr mcp;
};

