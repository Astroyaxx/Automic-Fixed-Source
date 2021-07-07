// i dont use this on growmmfi or anything its really simple and doesnt have any latest fixes or subserver system with synchronization, its literally just using multiple peers/hosts to connect.
#include "utils.hpp"
#include "ENetClient2.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>


const std::vector<std::string> utils::explode(const std::string& s, const char& c)
{
	std::string buff{ "" };
	std::vector<std::string> v;

	for (auto n : s)
	{
		if (n != c) buff += n; else
			if (n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if (buff != "") v.push_back(buff);

	return v;
}

std::string utils::gen_random(const int len) {
    std::string tmp_s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEF";

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) 
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    
    return tmp_s;
}

const void utils::ENetSend(ENetPeer* peer, char* data, int dataLen, int flags)
{
	if (peer)
	{
		if (peer->state != ENET_PEER_STATE_CONNECTED)
			return;
			
		int status = enet_peer_send(peer, 0, enet_packet_create(data, dataLen, flags));
	}
}

const void utils::ENetSend(ENetPeer* peer, ENetPacket* packet)
{
	if (peer)
	{
		if (peer->state != ENET_PEER_STATE_CONNECTED)
		{
			enet_packet_destroy(packet);
			return;
		}

		int status = enet_peer_send(peer, 0, packet);
		if (status != 0)
			printf("Error while enet_peer_send, error code: %d\n", status);
	}
	else {
		enet_packet_destroy(packet);
	}
}

const void utils::SendPacket(ENetPeer* peer, std::string text, int messageType, ENetHost* host)
{

	int textLen = text.length();
	text.resize(textLen + 5);

	char* textPtr = (char*)text.c_str();
	memmove(textPtr + 4, textPtr, textLen); // use memmove because the buffer we are working with is the same ptr as we are copying, and we don't want overlapping caused by memcpy to corrupt the data
	memcpy(textPtr, &messageType, 4);


	if (!host && peer)
		utils::ENetSend(peer, textPtr, textLen + 5);
	else if (host)
		utils::ENetBroadcast(host, textPtr, textLen + 5);
}

const void utils::SendPacketRaw(ENetPeer* peer, TankPacketStruct* raw, int flags)
{
	char packet[60] = { 0 };
	*(int*)packet = 4;
	memcpy(packet + 4, raw, 56);
	

	utils::ENetSend(peer, packet, 60, flags);
	//enet_host_flush(peer->host);
}

const void utils::ENetBroadcast(ENetHost* host, char* data, int dataLen, int flags)
{
	if (host)
	{
		enet_host_broadcast(host, 0, enet_packet_create(data, dataLen, flags));
	}
}

const void utils::SendHello(ENetPeer* peer)
{
	char hello[5];
	hello[0] = 1;
	ENetSend(peer, hello, 5);
}

const std::string utils::CreateLogonPacket(ENetPeer* peer, void* data) {
	ClientData* cData = (ClientData*)data;

	return (std::string)*cData;
}
