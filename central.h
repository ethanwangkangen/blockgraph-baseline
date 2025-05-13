#ifndef CENTRAL_H
#define CENTRAL_H

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
    void Setup();

    virtual void StartApplication();
    virtual void StopApplication();

  private:

    void ReceivePacket(Ptr<Socket> socket);

    void SendPacket(ApplicationPacket& packet, Ipv4Address ip, bool scheduled=false);

    void BroadcastPacket(ApplicationPacket& packet, vector<Ipv4Address v>);

    // Leader only
    void DisseminateData();
    
    void ProcessFollowerResponse(const string& payload, Ipv4Address senderAddr);

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
    vector<Ipv4Addresses> peers;

    // NS3 Variables
    bool running;
    stringstream debug_suffix; 
} 
