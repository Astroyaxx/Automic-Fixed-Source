#pragma once
#include <string>
#include "utils.hpp"
#include "TankPacketStruct.h"
#include <list>
#include <vector>

struct CL_Vec2f { 
    float x = 0.f, y = 0.f;

CL_Vec2f(float x = 0.f, float y = 0.f) { this->x = x; this->y = y; }
} __attribute__((packed));

struct Tile {
	//int padding;
	void* vtable;
	short fg, bg;
} __attribute__((packed));

struct NetAvatar {
    public:
        void* vtable;
        CL_Vec2f pos;
        char padding_1[24];
        std::string username;
        uint32_t netID;
        char padding_2[13];
        uint8_t facing_left;

} __attribute__((packed));

struct Camera {
    public:
        char padding_1[8];
        CL_Vec2f pos;
		char padding_2[16];
		float offset32;
		
} __attribute__((packed));


struct ItemInfo {
	public:
		int ItemID;
		uint8_t ItemType;
		char pad_1[11];
		std::string name;
		char pad[876];
} __attribute__((packed));

struct ItemInfoManager {
	public:
		char unk_1[4];
		std::vector<ItemInfo> items;
} __attribute__((packed));

struct WorldObject {
public:
    int vtable;//class?
    CL_Vec2f position;
    short itemID;
    uint8_t amount, invBits;
    uint32_t objectID;
    int pad [3];
} __attribute__((packed));

using SendPacket = void(*)(int, std::string, ENetPeer*);
using SendPacketRaw = void(*)(int, uint8_t*, size_t, void*, ENetPeer*, int flags);
using GetScreenSizeX = int(*)();
using GetScreenSizeY = int(*)();
using ProcessTankUpdatePacket = void(*)(void*, uint8_t*);
using HandleTrackPacket = void(*)(int, const char*);
using BaseAppDraw = void(*)(int);
using AppOnTouch = void*(*)(void*, void*, int, float, float, int);
using SetChosenItemID = int(*)(int, int);
using GameLogicComponentOnUpdate = void(*)(int);
using GetClient = void*(*)();
using GetLocalPlayer = NetAvatar*(*)(void*); //todo
using GetGameLogic = void*(*)();
using GetCamera = Camera*(*)();
using WorldCameraToScreen = void*(*)(float*, float*, float*);
using LevelTouchComponentHandleTouchAtWorldCoordinates = void(*)(void*, float*, bool);
using GetTileSafe = Tile*(*)(uint64_t, int, int);
using GetTileMap = void*(*)();
using GetTileAtWorldPos = void*(*)(void*, float*);
using GetItemInfoManager = ItemInfoManager*(*)();
using GetItemByIDSafe = ItemInfo*(*)(ItemInfoManager*, int);
using CanSeeGhosts = int(*)(int);
using OnSpawn = void(*)(void*, void*);
using NetAvatarSetSecurityStates = void(*)(NetAvatar*, bool, bool);
using AppGetVersionString = std::string(*)(void* /*App*/);
using GetEntityRoot = void*(*)();
using EntityGetEntityByName = void*(*)(void* /*this*/, std::string /*name*/);
using SetTextEntity = void(*)(void* /*this*/, std::string /*text*/);
using GetApp = void*(*)();
using OnlineSaveSettings = void(*)();
using WorldObjectMapGetObjectList = std::list<WorldObject>*(*)(void*);
using GetObjectMap = void*(*)();
using LogToConsoleSafe = void(*)(std::string);

inline SendPacket oSendPacket;
inline SendPacketRaw oSendPacketRaw;
inline GetScreenSizeX oGetScreenSizeX;
inline GetScreenSizeX oGetScreenSizeY;
inline ProcessTankUpdatePacket oProcessTankUpdatePacket;
inline HandleTrackPacket oHandleTrackPacket;
inline BaseAppDraw oBaseAppDraw;
inline AppOnTouch oAppOnTouch;
inline SetChosenItemID oSetChosenItemID;
inline GameLogicComponentOnUpdate oGameLogicComponentOnUpdate;
inline GetClient oGetClient;
inline GetLocalPlayer oGetLocalPlayer;
inline GetGameLogic oGetGameLogic;
inline GetCamera oGetCamera;
inline WorldCameraToScreen oWorldCameraToScreen;
inline LevelTouchComponentHandleTouchAtWorldCoordinates oLevelTouchComponentHandleTouchAtWorldCoordinates;
inline GetTileSafe oGetTileSafe; // unused for now
inline GetTileMap oGetTileMap;
inline GetTileAtWorldPos oGetTileAtWorldPos;
inline GetItemInfoManager oGetItemInfoManager;
inline GetItemByIDSafe oGetItemByIDSafe;
inline CanSeeGhosts oCanSeeGhosts;
inline OnSpawn oOnSpawn;
inline NetAvatarSetSecurityStates oNetAvatarSetSecurityStates;
inline AppGetVersionString oAppGetVersionString;
inline GetEntityRoot oGetEntityRoot;
inline EntityGetEntityByName oEntityGetEntityByName;
inline SetTextEntity oSetTextEntity;
inline GetApp oGetApp;
inline OnlineSaveSettings oOnlineSaveSettings;
inline WorldObjectMapGetObjectList oWorldObjectMapGetObjectList;
inline GetObjectMap oGetObjectMap;
inline LogToConsoleSafe oLogToConsoleSafe;

inline ENetPeer* GetPeer() {
	void* client = oGetClient(); // client addr
	if (!client) return NULL;


	return ((ENetPeer**)client)[20];
}