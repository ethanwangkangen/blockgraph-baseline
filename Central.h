#ifndef CENTRAL_H
#define CENTRAL_H

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/core-module.h"
#include "ns3/string.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-list.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/random-variable-stream.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/mobility-model.h"

#include "application_packet.h"
#include "transaction.h"
#include "block.h"
#include "blockgraph.h"
#include "b4m_traces.h"

#include <vector>
#include <utility>
#include <random>
#include <limits>
#include <queue>
#include <math.h>
#include <unordered_map>

#include "configs.h"

using namespace ns3;
using namespace std;

class Central : public Application{
  public:
    /**
     * Constructors and destructor
     */
    Central();
    ~Central();

  public:
    void SetUp(Ptr<Node> node, vector<Ipv4Address> peers);

    virtual void StartApplication();
    virtual void StopApplication();

    static TypeId GetTypeId(void);

    void ReceiveNewTopology(vector<pair<int, Ipv4Address>> new_group); // For mobility module to call

    Ptr<Central> GetCentral(int nodeId);

    void debug(string suffix);
    
    void GenerateResults();
  private:

    void ReceivePacket(Ptr<Socket> socket);

    void SendPacket(ApplicationPacket& packet, Ipv4Address ip, bool scheduled=false);

    void BroadcastPacket(ApplicationPacket& packet);

    Ipv4Address GetIpAddressFromId(int id);

    int GetIdFromIp(Ipv4Address ip);

    // Leader only
    void DisseminateData();

    void SendData(float SizeOfData, Ipv4Address destAddr);

    void RetransmitData(int term, int increment_count);

    void ProcessFollowerResponse(ApplicationPacket& p, Ipv4Address senderAddr);

  private:
    // Leader specific variables:i
    bool isLeader; // Only one leader in the whole network.
    int current_term; // Term of the current leader
    vector<pair<bool,int>> allNodes; // For leader to keep track of which nodes
                              // are in current group and have responded this term
                              // Pair is <isInCurrentGroup, latestTermReplied>..

    float sizeOfData; // Current size of data to send
    

    // General variables
    Ptr<Socket> recv_sock;
    Ptr<Node> node;
    vector<Ipv4Address> peers;

    // NS3 Variables
    bool running;
    stringstream debug_suffix; 
}; 
#endif
