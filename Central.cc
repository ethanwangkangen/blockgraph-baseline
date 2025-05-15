#include "Central.h"
#include <iomanip>
#include <vector>
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("Central");
NS_OBJECT_ENSURE_REGISTERED(Central);

TypeId Central::GetTypeId(){
  static TypeId tid = TypeId("Central")
    .SetParent<Application>()
    .SetGroupName("Application")
    .AddConstructor<Central>()
    ;
  return tid;
}

// Constructor
Central::Central() {
  isLeader = false;
  current_term = 0;
  sizeOfData = 50000; // Todo: change this
  // Set up in SetUp()
  allNodes = vector<pair<bool,int>>();
  recv_sock = 0; 
  node = Ptr<Node>();
  peers = vector<Ipv4Address>();
  running = false;
}

Central::~Central(){
}

/**
 * Initialise node, peers, recv_sock, allNodes array.
 */
void Central::SetUp(Ptr<Node> node, vector<Ipv4Address> peers){
  this->peers = peers;
  this->node = node;
  allNodes.assign(peers.size(), make_pair(false,0));

  if (node->GetId() == 1) { // Does this work?
    isLeader = true;
  } 

  if (!recv_sock){
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    recv_sock = Socket::CreateSocket(node, tid);
  }

  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 81);

  recv_sock->Bind(local);
  recv_sock->SetRecvCallback(MakeCallback(&Central::ReceivePacket, this));

}

void Central::StartApplication(){
  running = true;
  debug_suffix.str("");
  debug_suffix<< "Start B4Mesh on node : " << node->GetId() << endl;
  debug(debug_suffix.str()); 
  Simulator::ScheduleNow(&Central::DisseminateData, this);
}

void Central::StopApplication(){
  running = false;
}

// Either follower receiving entire data from leader, or leader receiving a response.
void Central::ReceivePacket(Ptr<Socket> socket){
  Ptr<Packet> packet;
  Address from;
  
  while (packet=socket->RecvFrom(from)){
    if (InetSocketAddress::IsMatchingType(from)){
      InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(from);
      Ipv4Address ip = iaddr.GetIpv4();


      debug_suffix.str("");
      debug_suffix << " Received Packet : New packet of " << packet->GetSize() << "B from Node " << ip;
      debug(debug_suffix.str());
      
      string parsedPacket;
      char* packetInfo = new char[packet->GetSize()];
      packet->CopyData(reinterpret_cast<uint8_t*>(packetInfo),
                    packet->GetSize());
      string parsed_packet = string(packetInfo, packet->GetSize());
      delete[] packetInfo;

      try{
        ApplicationPacket p(parsed_packet);

     	if (p.GetService() == ApplicationPacket::DATA){
          // Follower received a message from leader. Need to send reply/ack.
          debug_suffix.str("");
	  debug_suffix << "Follower received DATA of size " << p.GetSize()
		  << " from " << GetIdFromIp(ip) << " Sending reply"<< endl;
	  debug(debug_suffix.str());

      	  int term = p.GetTerm();
	  ApplicationPacket reply(term);
	  SendPacket(reply, ip, false);

	} else if (p.GetService() == ApplicationPacket::REPLY) {
          // If follower sending reply, need to take note
          
          debug_suffix.str("");
	  debug_suffix << "Leader received REPLY from " << GetIdFromIp(ip) << endl;
	  debug(debug_suffix.str());

	  ProcessFollowerResponse(p, ip);
	}

      } catch(const exception&e) {
        cerr <<e.what() << '\n';
        return;
      }
    }
  }

}

void Central::SendPacket(ApplicationPacket& packet, Ipv4Address ip, bool scheduled){
  if (running == false || ip == GetIpAddressFromId(node->GetId())){ // Don't send packet to self
    return;
  }

  debug_suffix.str("");
  debug_suffix << GetIdFromIp(ip) << " Sending packet of size " << packet.GetSize()
  	<< " to " << GetIdFromIp(ip) << endl;
  debug(debug_suffix.str());

  Ptr<Packet> pkt = Create<Packet>((const uint8_t*)(packet.Serialize().data()), packet.GetSize());

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> source = Socket::CreateSocket (node, tid);
  InetSocketAddress remote = InetSocketAddress (ip, 81);
  source->Connect(remote);

  int res = source->Send(pkt);
  source->Close();

  if (res > 0){
  } else {
  }
} 

// Need to modify this to only send to nodes in the group.
// Change to broadcast properly
void Central::BroadcastPacket(ApplicationPacket& packet){

  debug_suffix.str("");
  debug_suffix << node->GetId() << " Broadcasting packet of size " 
	  << packet.GetSize() << " to all nodes within group." << endl;
  debug(debug_suffix.str());

  if (!running) {
    return;
  }
  for (auto& ip : peers){
    if (allNodes[GetIdFromIp(ip)].first == true){ // if node is in group
      debug_suffix.str("");
      debug_suffix << "Broadcasting to: " << GetIdFromIp(ip) << endl;
      debug(debug_suffix.str());

      SendPacket(packet, ip, false);
    }
  } 

}

/**
 * Leader only. Broadcast the entire dataset at regular intervals.
 * Increment size of dataset, sizeOfData
 * Call on the function that starts to check which followers have replied, and
 * send message to those who haven't.
 *
 */
void Central::DisseminateData(){

  if (running==false || !isLeader){
    return;
  }

  sizeOfData += 100; // Todo: change this

  current_term++;


  debug_suffix.str("");
  debug_suffix << node->GetId() << " Disseminating data " << sizeOfData
  	<< " at term " << current_term << endl;
  debug(debug_suffix.str());

  // Create data packet
  ApplicationPacket p(current_term, sizeOfData);

  // Broadcast the packet. BroadcastPacket() takes care of handling which nodes
  // to send to (only those in current partition)
  BroadcastPacket(p);

  // Start retransmissions for this cycle.
  Simulator::Schedule(Seconds(2), &Central::RetransmitData, this, current_term, 1);

  // Todo: change interval to a variable
  Simulator::Schedule(Seconds(50), &Central::DisseminateData, this); 
}

/**
 * Within an interval (for one dataset size), keep retransmitting until all followers
 * get it, or max increment hit.
 *
 */
void Central::RetransmitData(int term, int increment_count){

  if (!running || !isLeader){
    return;
  }

  if (increment_count >= 20) { // Todo: implement this as a max increment
    return;
  }

  
  debug_suffix.str("");
  debug_suffix << node->GetId() << " Check for retransmittion at term " << term 
  	<< " increment " << increment_count << endl;
  debug(debug_suffix.str());

  bool requireRetransmission = false;

  
  for (size_t i = 0; i < allNodes.size(); ++i) {
    bool inGroup = allNodes[i].first;
    int latestTermReplied = allNodes[i].second;

    // If node is in group, check last term
    if (i != node->GetId() && inGroup && (latestTermReplied < current_term)){ 
	    // Node is in current group
      debug_suffix.str("");
      debug_suffix << "Retransmitting to " << i;
      debug(debug_suffix.str());

      SendData(sizeOfData, GetIpAddressFromId(i)); 
      requireRetransmission = true;
    }
  }

  // If all followers have responded for this term, don't need retransmit already.
  if (!requireRetransmission){
    return;
  }
  // Todo; Decide on interval between retransmissions
  Simulator::Schedule(Seconds(2), &Central::RetransmitData, this, term, increment_count+1);
}

Ipv4Address Central::GetIpAddressFromId(int id){
  Ipv4Address ipv4 = peers[id];
  return ipv4;
}

int Central::GetIdFromIp(Ipv4Address ip) {
  for (unsigned int i=0; i< peers.size(); i++){
    if (peers[i]==ip) {
      return i;
    }
  }
  return -1;
}

/**
 * Leader only. eend DATA packet of size sizeOfData to a specific follower node.
 */
void Central::SendData(float sizeOfData, Ipv4Address destAddr){
  
  debug_suffix.str("");
  debug_suffix << node->GetId() << " Sending data " << sizeOfData << " at term "
	<< current_term << " to node " << GetIdFromIp(destAddr) << endl;
  if (!running || !isLeader) {
    return;
  }
  // Create data packet p of size sizeOfData, send to destAddr
  ApplicationPacket p(current_term, sizeOfData);
  SendPacket(p, destAddr, false);
}


void Central::ProcessFollowerResponse(ApplicationPacket& p, Ipv4Address senderAddr){
  if (!running || !isLeader) {
    return;
  }
  int term = p.GetTerm();
  int followerId = GetIdFromIp(senderAddr);
  
  // Record that this follower replied for this term.
  allNodes[followerId].second = term;
}

// Todo: some callback function for mobility/group discovery model to update nodes in group
// Group is { [node id, ip address]... }
void Central::ReceiveNewTopology(vector<pair<int, Ipv4Address>> new_group) {

  if (!running) {
    return;
  }
  // Reset the status of allNodes
  for (unsigned int i = 0; i < allNodes.size(); ++i) {
    allNodes[i].first = false; // Set all to not be in current group
  }

  // Then add those in the new group
  // for (&pair :: new_group) {
  //   int id = pair.first;
  //   allNodes[i].first = true;
  // }

  for (auto& [id, ip] : new_group) {
    if (id >= 0 && id < (int)allNodes.size()) {
      allNodes[id].first = true;
    }
  }

}	

Ptr<Central> Central::GetCentral(int nodeId){
  Ptr<Application> app = ns3::NodeList::GetNode(nodeId)->GetApplication(0);
  Ptr<Central> centralApp = app->GetObject<Central>();
  return centralApp;
}


void Central::debug(string suffix){
  std::cout << Simulator::Now().GetSeconds() << "s: Central : Node " << node->GetId() <<
      " : " << suffix << endl;
  debug_suffix.str("");
 
}
