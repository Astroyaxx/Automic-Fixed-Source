#include "ENetClient2.h"
#include "GTEvents.h"
#include "utils.hpp"
#include <ostream>
#include <fstream>
#include <iostream>
#include "TankPacketStruct.h"

ENetClient2* g_botClient = NULL;

void GTEvents::ProcessTankUpdatePacket(ENetPeer* peer, void* rawData)
{
	//printf("ProcessTankUpdatePacket, received packet with type: %d\n", raw->header.packetType);

    
    TankPacketStruct* raw = (TankPacketStruct*)rawData;
   
	switch (raw->packetType) 
	{
	case PACKET_STATE:
		break;
	case PACKET_CALL_FUNCTION:
	{
		VariantList* vList = new Variant::VariantList(rawData);
		GTEvents::ProcessVariantCall(peer, vList);

		VariantList::Kill(vList);
		break;
	}
	case PACKET_TILE_APPLY_DAMAGE:
	{
		
		break;
	}
	case PACKET_SEND_MAP_DATA:
	{
		break;
	}
	default:
		//LOGC << "Unknown packet type: " << (int)raw->header.packetType << ENDL;
		break;
	}
}

void GTEvents::ProcessServerHello(ENetPeer* peer)
{
	//LOG("Processing server-hello, will send login data as response...");

	if (!peer->data) return;

	ClientData* cData = peer_info(peer);
	cData->winkey = utils::gen_random(32);

	std::string logonStr = utils::CreateLogonPacket(peer, peer->data);
	//LOGI("Responding to server hello, logon detail: %s", logonStr.c_str());
	utils::SendPacket(peer, logonStr, 2);
}

void GTEvents::ProcessTextPacket(ENetPeer* peer, std::string text, bool isGame)
{
	// Message Type 3 is dedicated to slightly different actions than normal text packet iirc.
	int msgType = isGame ? 3 : 2;

#ifdef ISDEBUGBUILD
	LOGI("[?] Text is here: %s", text.c_str());
#endif
    //LOGI("[?] Text is here: %s", text.c_str());
	//LOGC << "[?] Text is here: " << text << ENDL;

	if (isGame) {

	}
	else {

	}
}

void GTEvents::ProcessVariantCall(ENetPeer* peer, VariantList* vList)
{
	short vc = vList->GetCount();

	if (vc < 1) {
		//LOGC << "ERROR at GTEvents::ProcessVariantCall >> illegal vlist size: " << vc <<  " aborting!" << ENDL;
		return;
	} //nerde http request edilcek

	std::string fn = vList->GetFuncName();
	//LOGC << " Got Function Call: " << fn << ENDL;
	ClientData* cData = peer_info(peer);

	std::string argInStr;

	if (fn == "OnSendToServer") { // send to different growtopia subserver
		std::vector<std::string> ipDoorID = utils::explode(vList->GetFuncArg(4), '|');

		std::string ip = ipDoorID[0];
		int gtport = vList->GetFuncArg(1);
		int token = vList->GetFuncArg(2);
		int userID = vList->GetFuncArg(3);
		int lmode = vList->GetFuncArg(5);
		
		
		//LOGI("Switching server, explicit details: \n\t=> %s\n\t=> %d\n\t=> %d\n\t=> %d\n\t=> %d\n", ip.c_str(), gtport, token, userID, lmode);

		
		cData->token = token;
		cData->userID = userID;
		cData->lmode = lmode;
		cData->ip = ip;
		cData->port = gtport;
        cData->reconnect = true;

		if (token <= -1) {
			cData->ip = "";
			cData->port = 17200;
		}

		if (g_botClient) {
			enet_peer_disconnect(peer, 0);
		}
		
	}
	else if (fn == "OnRemove") {
		std::string removeStr = vList->GetFuncArg(1);
		size_t del = removeStr.find('|');
		if (del == -1) return;

		std::string key = removeStr.substr(0, del);
		std::string value = removeStr.substr(del + 1);

		peer_info(peer)->netAvatarList.Remove(atoi(value.c_str()));
	}
	else if (fn == "OnSpawn") {
		std::string spawnStr = vList->GetFuncArg(1);
		NetAvatar2 netAvatar;

		std::vector<std::string> lines = utils::explode(spawnStr, '\n');
		for (const auto& line : lines) 
		{
			size_t del = line.find('|');
			if (del == -1) continue;

			std::string key = line.substr(0, del);
			std::string value = line.substr(del + 1);

			if (key == "netID") {
				netAvatar.netID = atoi(value.c_str());
			}
			else if (key == "userID") {
				netAvatar.userID = atoi(value.c_str());
			}
			else if (key == "name") {
				netAvatar.name = value;
				//LOGC << "Adding player with name: " << netAvatar.name << " into NetAvatarList!" << ENDL;
			}
			else if (key == "type" && value == "local") {
				netAvatar.local = true;
				peer_info(peer)->localAvatar = netAvatar;
				return;
			}
		}
		peer_info(peer)->netAvatarList.Add(netAvatar);
	}
	else if (fn == "OnSuperMainStartAcceptLogonHrdxs47254722215a") {
		peer_info(peer)->reconnect = true;
		//OnSuperMainStartAcceptLogonHrdxs47254722215a
		//LOGC << "[!] Logged in as: " << peer_info(peer)->tankIDName <<  " userID: " << peer_info(peer)->userID << ENDL;
		//burdan name/pass alýrýz logini acceptler ise veya acceptlemedende alabiliyosak öylede olr böyle daha iyi yanýþ þifre gelmez ama þimdi adam yanlýslýkla alt hesabýyla falan bence girdigi hepsi gelse daha iyi olur ya

		utils::SendPacket(peer, "action|enter_game\n", 2);
		//utils::SendPacket(peer, "action|join_request\nname|NABSI2875", NET_MESSAGE_GAME_MESSAGE);
	}
	else if (fn == "OnRequestWorldSelectMenu") {
		peer_info(peer)->netAvatarList.Clear();
		//utils::SendPacket(peer, "action|join_request\nname|NABSI2875", NET_MESSAGE_GAME_MESSAGE);
	}
	else {
		//LOGC << "Unknown varfunction call: " << fn << ENDL;
	}
}