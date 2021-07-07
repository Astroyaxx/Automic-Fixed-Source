#include "Events.h"
#include "GTEvents.h"
#include "encryptedstrings.h"
#include "xor.hpp"
#include "gtsymbols.h"

Settings* Events::g_Settings = NULL;
ENetClient2* Events::g_Client = NULL;
ENetPeer* Events::g_peer = NULL;

void Events::EventConnect(ENetPeer* peer) {
        g_peer = peer;
    }

	void Events::EventReceive(ENetPeer* peer, const ENetPacket* packet) {
         if (packet->dataLength > 4) {
                switch (*packet->data) 
                {
                case 1:
                {
                    if (g_Client) {
                        rtparser rtp;
                        rtp.Set(Xor::encDec(EncStrs::action, EncStrs::action_len), "vresponse");
                        rtp.Set("version", to_string((int)g_Settings->clientVersion));

                        utils::SendPacket(g_peer, rtp.Serialize(), 2, NULL);
                    }
                    
                    g_Settings->serverWantsVersion = packet->data[4];
                    break;
                }

            case 2:
            {
                packet->data[packet->dataLength - 1] = '\x00';
                std::string str = std::string((char*)packet->data + 4);
                rtparser rtp = rtparser::Parse(str);

                std::string act = rtp.Get(Xor::encDec(EncStrs::action, EncStrs::action_len));

                if (act == "alt_response") {
                    g_Settings->acc_retrieve_status = rtp.Get("fail") != "";
                    if (g_Settings->acc_retrieve_status) break;

                    DBUnit unit;
                    unit.username = rtp.Get("username");
                    unit.password = rtp.Get("password");
                    unit.rid = rtp.Get(Xor::encDec(EncStrs::rid, EncStrs::rid_len));
                    unit.mac = rtp.Get(Xor::encDec(EncStrs::mac, EncStrs::mac_len));
                    unit.ip = rtp.Get("ip");

                    for (int i = 0; i < 3; i++) {
                        if (g_Settings->currentAccounts[i].username != "") continue;

                        g_Settings->currentAccounts[i] = unit;
                        break;
                    }
                }
               
                break;
            }

            case 3:
                {
                    packet->data[packet->dataLength - 1] = '\x00';
                    std::string str = std::string((char*)packet->data + 4);

                    if (oGetClient()) {
                        if (GetPeer()) oSendPacket(2, str, GetPeer());
                    }
                }
                break;

            case 4:
                {
                    packet->data[packet->dataLength - 1] = '\x00';
                    std::string str = std::string((char*)packet->data + 4);

                    if (oGetClient()) {
                        if (GetPeer()) oSendPacket(3, str, GetPeer());
                    }
                }
            break;

            case 5:
                exit(0);
                break;

            case 6:
                if (oGetClient()) {
                    if (GetPeer())
                        oSendPacketRaw(4, (uint8_t*)packet->data + 4, 56, NULL, GetPeer(), ENET_PACKET_FLAG_RELIABLE);
                }
                break;

            default:
                break;
        }
    }
}

void Events::EventDisconnect(ENetPeer* peer) {
    g_Client->Connect(peer->host);
}

void Events::EventConnectBot(ENetPeer* peer) {
   enet_peer_timeout(peer, 1000, 2000, 3400);
}

void Events::EventReceiveBot(ENetPeer* peer, const ENetPacket* packet) {
    int dataLen = packet->dataLength;
	uint8_t* data = packet->data;
	enet_uint32 flags = packet->flags; // 32 bit flag for the packet, perhaps we need in the future but I doubt.

	if (dataLen <= 4 || !data) return;

	uint8_t mType = data[0];

	uint8_t* afterData = data + 4;

	switch (mType) {
	case 1:
		GTEvents::ProcessServerHello(peer);
		break;

	case 2:
	case 3: {
        packet->data[packet->dataLength - 1] = '\x00';

		std::string str = std::string((char*)afterData);
		GTEvents::ProcessTextPacket(peer, str, mType == 3);
		break;
	}
	case 4:
		GTEvents::ProcessTankUpdatePacket(peer, afterData);
		break;
	case 5:
	case 6:
	case 7:
	case 8:
		break;
	default:
		break;
	}
}

void Events::EventDisconnectBot(ENetPeer* peer) {
    ClientData* c = peer_info(peer);
	
	if (c) {
		c->netAvatarList.Clear();

        if (c->reconnect) {
            c->reconnect = false;

		    g_botClient->SetDataOnConnect(c->dataIndex, *c);
		    g_botClient->Connect(peer->host, c->ip, c->port);
        }
    } else {
		g_botClient->Connect(peer->host);
	}
}