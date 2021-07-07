#pragma once
#include "rtparser.h"
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"
#include "Includes/Logger.h"

enum {
    PACKET_STATE = 0,
    PACKET_CALL_FUNCTION,
    PACKET_UPDATE_STATUS,
    PACKET_TILE_CHANGE_REQUEST,
    PACKET_SEND_MAP_DATA,
    PACKET_SEND_TILE_UPDATE_DATA,
    PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE,
    PACKET_TILE_ACTIVATE_REQUEST,
    PACKET_TILE_APPLY_DAMAGE,
    PACKET_SEND_INVENTORY_STATE,
    PACKET_ITEM_ACTIVATE_REQUEST,
    PACKET_ITEM_ACTIVATE_OBJECT_REQUEST,
    PACKET_SEND_TILE_TREE_STATE,
    PACKET_MODIFY_ITEM_INVENTORY,
    PACKET_ITEM_CHANGE_OBJECT,
    PACKET_SEND_LOCK,
    PACKET_SEND_ITEM_DATABASE_DATA,
    PACKET_SEND_PARTICLE_EFFECT,
    PACKET_SET_ICON_STATE,
    PACKET_ITEM_EFFECT,
    PACKET_SET_CHARACTER_STATE,
    PACKET_PING_REPLY,
    PACKET_PING_REQUEST,
    PACKET_GOT_PUNCHED,
    PACKET_APP_CHECK_RESPONSE,
    PACKET_APP_INTEGRITY_FAIL,
    PACKET_DISCONNECT,
    PACKET_BATTLE_JOIN,
    PACKET_BATTLE_EVEN,
    PACKET_USE_DOOR,
    PACKET_SEND_PARENTAL,
    PACKET_GONE_FISHIN,
    PACKET_STEAM,
    PACKET_PET_BATTLE,
    PACKET_NPC,
    PACKET_SPECIAL,
    PACKET_SEND_PARTICLE_EFFECT_V2,
    GAME_ACTIVE_ARROW_TO_ITEM,
    GAME_SELECT_TILE_INDEX
};

struct TankPacketStruct 
{
#pragma pack (push,1)
    uint8_t packetType = 0;
    uint8_t padding1 = 0, padding2 = 0, padding3 = 0;
    int NetID = 0;
    int secondaryNetID = 0;
    int characterState = 0;
    float padding4 = 0;
    int value = 0;
    float x = 0, y = 0;
    int XSpeed = 0, YSpeed = 0;
    int padding5 = 0;
    int punchX = 0, punchY = 0;
    uint32_t extDataSize = 0;
#pragma pack (pop)
};

inline const std::string LOG_TANKPACKETSTRUCT(TankPacketStruct* tStruct) {
    rtparser rtp{};
	rtp.Set("packetType", to_string((int)tStruct->packetType));
	rtp.Set("padding1", to_string((int)tStruct->padding1));
	rtp.Set("padding2", to_string((int)tStruct->padding2));
	rtp.Set("padding3", to_string((int)tStruct->padding3));
	rtp.Set("netID", to_string((int)tStruct->NetID));
	rtp.Set("secondNetID", to_string((int)tStruct->secondaryNetID));
	rtp.Set("characterState", to_string((int)tStruct->characterState));
	rtp.Set("padding4", to_string((int)tStruct->padding4));
	rtp.Set("value", to_string((int)tStruct->value));
	rtp.Set("x", to_string((int)tStruct->x));
	rtp.Set("y", to_string((int)tStruct->y));
	rtp.Set("XSpeed", to_string((int)tStruct->XSpeed));
	rtp.Set("YSpeed", to_string((int)tStruct->YSpeed));
	rtp.Set("padding5", to_string((int)tStruct->padding5));
	rtp.Set("PunchX", to_string((int)tStruct->punchX));
	rtp.Set("PunchY", to_string((int)tStruct->punchY));

    std::string str = rtp.Serialize();

    LOGI("%s", str.c_str());
    return str;
}
