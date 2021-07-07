#pragma once
#include "Includes/Logger.h"
#include "utils.hpp"
#include "rtparser.h"

namespace Events {
    extern Settings* g_Settings;
    extern ENetClient2* g_Client;
    extern ENetPeer* g_peer;

	void EventConnect(ENetPeer* peer);

	void EventReceive(ENetPeer* peer, const ENetPacket* packet);

	void EventDisconnect(ENetPeer* peer);

    void EventConnectBot(ENetPeer* peer);

    void EventReceiveBot(ENetPeer* peer, const ENetPacket* packet);

    void EventDisconnectBot(ENetPeer* peer);
}