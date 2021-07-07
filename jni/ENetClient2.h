#pragma once
#include <enet/include/enet.h>
#include <string>
#include "utils.hpp"
#include "rtparser.h"
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"
#include <vector>

//imgui diye directory var orda olmasýn?
// so it wont interfere with current varnames like NetAvatar exists alrd...
typedef void EventCallOnConnect(ENetPeer*);
typedef void EventCallOnDisconnect(ENetPeer*);
typedef void EventCallOnReceive(ENetPeer*, ENetPacket*);

struct NetAvatar2 {
	bool added = false;
	int netID, userID;
	std::string name;
	bool local;
};

class NetAvatar2List {
public:
	std::vector<NetAvatar2> netAvatars;

	const size_t GetCount() { return netAvatars.size(); }
	const NetAvatar2 Get(int netID) {
		NetAvatar2 netAv;
		for (const auto netavatar : netAvatars)
		{
			if (netavatar.netID == netID)
				return netavatar;
		}
		return netAv;
	}
	const NetAvatar2 GetByUserID(int userID) {
		NetAvatar2 netAv;
		for (const auto netavatar : netAvatars)
		{
			if (netavatar.userID == userID)
				return netavatar;
		}
		return netAv;
	}
	const void Add(NetAvatar2 av) {
		NetAvatar2 netAv = Get(av.netID);
		if (netAv.added) return; // Same NetID exists already? Remove first.
		NetAvatar2 netAv2 = Get(av.userID);
		if (netAv2.added) return; // Same UserID is also illegal.

		av.added = true;
		netAvatars.push_back(av);
	}
	const void Remove(int netID) {
		int ind = -1;
		for (int i = 0; i < GetCount(); i++) 
		{
			NetAvatar2 av = netAvatars[i];
			if (av.netID == netID)
			{
				ind = i;
				break;
			}
		}

		if (ind > -1) netAvatars.erase(netAvatars.begin() + ind);
	}
	const void Clear() { netAvatars.clear(); }
};


struct ClientData {
	std::string currentWorld;

	bool inWorld = false;
	bool isVerified = false;
	bool beSynced = false; // is Multibotting enabled?
	bool ready = false; // Is it ready?
	int orderId;
	bool reconnect = false;

	int linkedWithUserID = -1;

	NetAvatar2 localAvatar;
	NetAvatar2List netAvatarList;

	int dataIndex = -1;
	std::string ip;
	uint16_t port = 17093;

	std::string tankIDName, tankIDPass,
		requestedName = "LuckDar",
		meta = "undefined", country = "us",
		mac = "02:EA:A9:34:FA:64", doorID, rid = "0130F9FE00973D2704ED799509839267", winkey = "131D903E4C01462FD6D04187433773F4";

	int userID = 0, token = 0;
	float game_version = 4.69;
	uint8_t f = 1, lmode = 0, cbits = 0, age = 21, GDPR = 1, platformID = 0, deviceVersion = 0;
	short protocol = 130;
	int hash = -700085872, hash2 = -1304654494, fhash = -716928004;

	ClientData() {}
	ClientData(ClientData* cData)
	{
		dataIndex = cData->dataIndex;
		tankIDName = cData->tankIDName;
		tankIDPass = cData->tankIDPass;
		userID = cData->userID;
		token = cData->token;
	}

	operator std::string() {
		std::string l;
		rtparser rtp;
		if (tankIDName != "") {
			rtp.Set("tankIDName", tankIDName);
			rtp.Set("tankIDPass", tankIDPass);
		}
		//yada burdamý alsak la sal
		rtp.Set("requestedName", requestedName);
		rtp.Set("f", to_string((int)f));
		rtp.Set("protocol", to_string((int)protocol));
		rtp.Set("game_version", to_string((float)game_version));
		if (token > -1) rtp.Set("lmode", to_string((int)lmode));

		rtp.Set("cbits", to_string((int)cbits));
		rtp.Set("player_age", to_string((int)age));
		rtp.Set("GDPR", to_string((int)GDPR));
		rtp.Set("hash2", to_string((int)hash2));
		rtp.Set("meta", meta);
		rtp.Set("fhash", to_string((int)fhash));
		rtp.Set("rid", rid);
		rtp.Set("platformID", to_string((int)platformID));
		rtp.Set("deviceVersion", to_string((int)deviceVersion));
		rtp.Set("country", country);
		rtp.Set("hash", to_string((int)hash));
		rtp.Set("mac", mac);

		if (token > 0) {
			rtp.Set("user", to_string((int)userID));
			rtp.Set("token", to_string((int)token));
		}

		if (doorID != "")
			rtp.Set("doorID", doorID);

		rtp.Set("wk", winkey);
		rtp.Set("zf", to_string(-2144598728));

		return rtp.Serialize();
	}
};

class ENetClient2 {
public:
	static void Kill(ENetClient2* client);
	void SetDataOnConnect(const int id, ClientData& cData);
	ClientData* GetInitialData(const int id);
	ENetHost* GetClient(int hostID);
	ENetHost* SetupHost(int hostID, const char* hostName, enet_uint16 port, 
		int maxOutgoingConnections = 64, int channels = 2, int maxIncomingBandwidth = 0, int maxOutgoingBandwidth = 0, int protocolRevision = 1);

	void SetConnectEvent(void* funcRef);
	void SetReceiveEvent(void* funcRef);
	void SetDisconnectEvent(void* funcRef);
	void InitializeEvents();

	void Disconnect(int hostID);
	bool isConnected(int hostID);

	void BroadcastPacketRaw(TankPacketStruct* raw);

	ENetPeer* Connect(ENetHost* host, std::string ip = "", uint16_t cPort = 17091);
	ENetEvent poll(int hostID, int delay, void* server = NULL, ENetHost* anyHost = NULL); // client has the ability to look out for the internal server too.
	void destroy(); // delete self.

	std::string hostIP;
	enet_uint16 masterport = 17196, port;
	const int GetHostsSize() { return hosts.size(); }
	std::vector<ENetHost*> GetHosts();

	void BroadcastPacket(std::string text, int messageType = 2);

	ClientData defaultInitialConnectData;
private:
	std::vector<ClientData> initialConnectDatas; // ClientData that gets set when client connects for the first time.

	std::vector<ENetHost*> hosts;

	EventCallOnConnect* OnConnect = NULL;
	EventCallOnDisconnect* OnDisconnect = NULL;
	EventCallOnReceive* OnReceive = NULL;
};
