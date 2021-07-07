#pragma once
#include <cstring>
#include <map>
#include <vector>
#include "TankPacketStruct.h"
#include "rtparser.h"
#include "ENetClient2.h"
#include "gtsymbols.h"

#define peer_info(peer) ((ClientData*)(peer->data))
typedef uint8_t u8;
typedef size_t sz;

struct DBUnit {
		std::string username = "", password, mac, rid, ip;

		DBUnit(std::string username = "", std::string password = "",
		std::string mac = "", std::string rid = "", std::string ip = "") {
			this->username = username;
			this->password = password;
			this->mac = mac;
			this->rid = rid;
			this->ip = ip;
		}
};

enum {
	FARMMODE_ONEBLOCK,
	FARMMODE_TWOBLOCK,
	FARMMODE_RAYMAN,
	FARMMODE_TRACTOR,
	FARMMODE_NUKER // circle
};

static struct Settings {

	bool botmirror = false;
	bool fast_drop = false;
	bool auto_collect = false;
	bool auto_place = false;
	bool auto_break = false;
	bool auto_leave = false;
	bool gold_hit = false;

	int pickup_range = 1;
	int place_item_id = 0;
	int break_item_id = 0;
	int characterState = 0;

	bool freecam = false;
    float DeltaTime[2];
    CL_Vec2f prev_touch_location;
	CL_Vec2f last_camera_pos;

	bool connectedToATOMICserver = false; // unused but still referenced in here I think, not sure. dont remove
	bool ghostsVisible = false;
	
	uint8_t farmMode = FARMMODE_ONEBLOCK;

	bool mstate = false;
	const uint8_t clientVersion = 5;
	uint8_t serverWantsVersion = clientVersion;

	rtparser accountData;
	bool acc_retrieve_status = false; // true = FAIL (due to too many requests in 1 hour)
	DBUnit currentAccounts[3];
	// for spoofing

	std::string gid = "", rid = "", mac = "", wk = "";

} settings;

namespace utils {
	const std::vector<std::string> explode(const std::string& s, const char& c);
	const void ENetSend(ENetPeer* peer, char* data, int dataLen, int flags = 1);
	const void ENetSend(ENetPeer* peer, ENetPacket* packet);
	const void SendPacket(ENetPeer* peer, std::string text, int messageType, ENetHost* host = NULL); // if want to broadcast, host shouldn't be NULL
	const void SendPacketRaw(ENetPeer* peer, TankPacketStruct* raw, int flags = 1);
	const void ENetBroadcast(ENetHost* host, char* data, int dataLen, int flags = 1);
	
	//const std::string CreateLogonPacket(ENetPeer* peer, Networking::ClientData* Data);
	const void SendHello(ENetPeer* peer);

	template<typename T>
	constexpr T GetStructure(uint8_t* data) {
		auto structure = *(T*)data; 
		data += sizeof(T);
		return structure;
    }

    const inline bool isInside(int circle_x, int circle_y, 
                   int rad, int x, int y) 
    { 
    // Compare radius of circle with distance  
    // of its center from given point 
        if ((x - circle_x) * (x - circle_x) + 
            (y - circle_y) * (y - circle_y) <= rad * rad) 
            return true; 
        else
            return false; 
    } 

	const std::string CreateLogonPacket(ENetPeer* peer, void* data);
	std::string gen_random(const int len);
}