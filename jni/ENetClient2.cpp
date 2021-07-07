#include "ENetClient2.h"
#include "Events.h"
#include "utils.hpp"

#include "Includes/Logger.h"

void ENetClient2::Kill(ENetClient2* client)
{
    //LOG("Killing ENetClient...");
    client->destroy();
    client = NULL;
    //LOG("ENetClient successfully killed!");
}

void ENetClient2::SetDataOnConnect(const int id, ClientData& cData)
{
    int count = initialConnectDatas.size();
    if (id >= count) {
        //LOG("(WARNING) Adding new initial connect data...");
        initialConnectDatas.push_back(cData);
    }
   

    if (id < initialConnectDatas.size()) { // check if it pushed successfully.
        ClientData* c = &initialConnectDatas[id]; // pointer access temporarily is fine as long as it's also just used when required and not globally declared or smth

        //if (c->dataIndex > -1)
            //LOG("(WARNING) Seems like client data was already set, overriding now...");

        cData.dataIndex = id;
        initialConnectDatas[id] = cData;

        /*LOG("Overwritten 'initialConnectData' [ClientData], new tankIDName/token will be logged here: %s/%d",
            initialConnectDatas[id].tankIDName.c_str(),
            initialConnectDatas[id].token);*/
    }
}

ClientData* ENetClient2::GetInitialData(const int id)
{
    //LOGC << "Get initial data, ID: " << id << " size: " << initialConnectDatas.size() << ENDL;
    if (id < 0)
        return &defaultInitialConnectData;
    else if (id >= initialConnectDatas.size())
        return NULL;
    
    return &initialConnectDatas[id];
}

ENetHost* ENetClient2::GetClient(int hostID)
{
    if (hostID >= hosts.size()) return nullptr;
    //if (!hosts[hostID]) LOG("(WARNING) => Be cautious, ENetClient2::host (ENetHost*) is NULL! Better set it now.");

    return hosts[hostID];
}

ENetHost* ENetClient2::SetupHost(int hostID, const char* hostName, enet_uint16 port, 
    int maxOutgoingConnections, int channels, int maxIncomingBandwidth, int maxOutgoingBandwidth, int protocolRevision)
{
    ENetHost* host = GetClient(hostID);

    if (!host) {
        //LOG("Setting up host...");

        host = enet_host_create(NULL /* create a client host */,
            maxOutgoingConnections /* only allow 1 outgoing connection */,
            channels /* allow up 2 channels to be used, 0 and 1 */,
            maxIncomingBandwidth /* assume any amount of incoming bandwidth */,
            maxOutgoingBandwidth /* assume any amount of outgoing bandwidth */);
        if (host == NULL)
        {
            //fprintf(stderr,
                //"An error occurred while trying to create an ENet client host.\n");
            return host;
        }


        this->hostIP = hostName;
        this->port = port;
        host->checksum = enet_crc32;
        host->usingNewPacket = protocolRevision;

        enet_host_compress_with_range_coder(host);

        if (hostID > -1)
            hosts.push_back(host);

        //LOG("Host has been set up! Protocol rev: %d", protocolRevision);
    }
    return host;
}

void ENetClient2::SetConnectEvent(void* funcRef)
{
    this->OnConnect = (EventCallOnConnect*)funcRef;
}

void ENetClient2::SetReceiveEvent(void* funcRef)
{
    this->OnReceive = (EventCallOnReceive*)funcRef;
}

void ENetClient2::SetDisconnectEvent(void* funcRef)
{
    this->OnDisconnect = (EventCallOnDisconnect*)funcRef;
}

void ENetClient2::InitializeEvents()
{
    //g_ENetClient = this;
}

bool ENetClient2::isConnected(int hostID) {
    ENetHost* host = hosts[hostID];

    ENetPeer* currentPeer;
    for (currentPeer = host -> peers;
         currentPeer < & host -> peers [host -> peerCount];
         ++ currentPeer)
    {
       if (currentPeer -> state == ENET_PEER_STATE_CONNECTED)
         return true;
    }
    
    return false;
}

void ENetClient2::Disconnect(int hostID) {
    ENetHost* host = hosts[hostID];

    ENetPeer* currentPeer;
    for (currentPeer = host -> peers;
         currentPeer < & host -> peers [host -> peerCount];
         ++ currentPeer)
    {   
        if (currentPeer -> state != ENET_PEER_STATE_CONNECTED)
            continue;

        if (currentPeer->data)
            peer_info(currentPeer)->reconnect = false;

        enet_peer_disconnect(currentPeer, 0);
    }
}

ENetPeer* ENetClient2::Connect(ENetHost* host, std::string ip, uint16_t port)
{
    ENetAddress address;
    ENetPeer* peer;
    /* Connect to some.server.net:1234. */
    bool useCustomData = (ip != "");

    enet_address_set_host(&address, useCustomData ? ip.c_str() : this->hostIP.c_str());
    address.port = useCustomData ? port : this->port;
    
#ifdef ISDEBUGBUILD
    LOGI("Connecting to %s:%d\n", useCustomData ? ip.c_str() : this->hostIP.c_str(), address.port);
#endif
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(host, &address, 2, 0);
    if (peer == NULL)
    {
        //LOGI("No peers available.");
        //fprintf(stderr,
            //"No available peers for initiating an ENet connection.\n");
    }
    return peer;
}

ENetEvent ENetClient2::poll(int hostID, int delay, void* server, ENetHost* anyHost)
{
    ENetEvent event;
    ENetHost* host = hostID < 0 ? anyHost : hosts[hostID];

    while (enet_host_service(host, &event, delay) > 0)
    {
        //if (server) ((ENetServer*)server)->poll(0);
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            event.peer->data = new ClientData(GetInitialData(hostID));
            enet_peer_timeout(event.peer, 1000, 10000, 10000);

            if (this->OnConnect) this->OnConnect(event.peer);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            
            if (this->OnReceive) this->OnReceive(event.peer, event.packet);
            //else LOG("WTF?! this->OnReceive was null!");
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            if (this->OnDisconnect) this->OnDisconnect(event.peer);
            if (event.peer->data) {
                delete (ClientData*)event.peer->data;
            }

            event.peer->data = NULL;
            break;
        }
    }

    return event;
}

void ENetClient2::destroy()
{
    for (auto& host : hosts) 
    {
        if (host) {
            //LOG("Destroying ENetClient2::host...");
            enet_host_destroy(host);
            host = NULL;
            //LOG("Destroyed ENetClient2::host!");
        }
    }

    hosts.clear();
    delete this;
}

std::vector<ENetHost*> ENetClient2::GetHosts()
{
    return hosts;
}

void ENetClient2::BroadcastPacketRaw(TankPacketStruct* raw) {
    char data[60];
    *(int*)data = 4;
    memcpy(data + 4, raw, 56);

    for (const auto& host : hosts) {
        if (!host) continue;

        utils::ENetBroadcast(host, data, 60);
    }
}

void ENetClient2::BroadcastPacket(std::string text, int messageType)
{
    for (const auto& host : hosts) {
        if (!host) continue;

        utils::SendPacket(NULL, text, messageType, host);
    }
}
