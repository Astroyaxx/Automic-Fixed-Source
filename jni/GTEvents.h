#pragma once
#include "VariantList.h"
#include "ENetClient2.h"

using namespace Variant;
extern ENetClient2* g_botClient; // nah lets use it as global link

namespace GTEvents {
	void ProcessTankUpdatePacket(ENetPeer* peer, void* rawData);
	void ProcessServerHello(ENetPeer* peer); // Login
	void ProcessTextPacket(ENetPeer* peer, std::string text, bool isGame = false); // isGame = if m type is 3
	void ProcessVariantCall(ENetPeer* peer, VariantList* vList);
	void ProcessTrackPacket(ENetPeer* peer, std::string track);
}