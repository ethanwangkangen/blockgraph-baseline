#include "Central.h"
#include <iomanip>
NS_LOG_COMPONENT_DEFINE("Central");
NS_OBJECT_ENSURE_REGISTERED(Central);

// Constructor
Central::Central() {
  isLeader = false;
  current_term = 0;
  //allNodes = ;
  
  recv_sock = 0; // Setup later
  node = Ptr<Node>();
  peers = vector<Ipv4Address>();

}

Central::~Central(){
}

void Central::SetUp(Ptr<Node> node, vector<Ipv4Address> peers){
  this->peers = peers;
  this->node = node;

  if (!recv_sock){
    TypeId tid = TypeId:LookupByName("ns3::UdpSocketFactory");
    recv_sock = Socket::CreateSocket(node, tid);
  }

  InetSocketAddress local = InetSocketAddress(Ipv4Address:GetAny(), 81);

  recv_sock->Bind(local);
  recv_sock->SetRecvCallback(MakeCallback(&Central::ReceivePacket, this));

}

void Central::StartApplication(){
  running = true;
  Simulator::ScheduleNow(&Central::DisseminateData, this);
}

// Either follower receiving entire data from leader, or leader receiving a response.
void Central::ReceivePacket(Ptr<Socket> socket){
  Ptr<Packet> packet;
  Address from;
  
  while (packet=socket->RecvFrom(from)){
    if (InetSocketAddress::IsMatchingType(from)){
      InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(from);
      Ipv4Address ip = iaddr.GetIpv4();

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
	  int term = p.GetTerm();
	  ApplicationPacket reply(term);
	  SendPacket(reply, ip, false);

	} else if (p.GetService() == ApplicationPacket::REPLY) {
          // If follower sending reply, need to take note
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
  if (running == false){
    return;
  }

  Ptr<Packet> pkt = Create<Packet>((const uint8_t*)(packet.Serialize().data()), packet.GetSize());

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> source = Socket::CreateSocket (node, tid);
  InetSocketAddress remote = InetSocketAddress (ip, 80);
  source->Connect(remote);

  int res = source->Send(pkt);
  source->Close();

  if (res > 0){
  } else {
  }
} 

// Need to modify this to only send to nodes in the group.
void Central::BroadcastPacket(ApplicationPacket& packet){
  for (auto& ip : peers){
    SendPacket(packet, ip, false);
  } 
}

/**
 * Broadcast the entire dataset at regular intervals.
 * Increment size of dataset, sizeOfData
 * Call on the function that starts to check which followers have replied, and
 * send message to those who haven't.
 *
 *
 */
void Central::DisseminateData(){
  if (running==false || !isLeader){
    return;
  }

  sizeOfData += 100; // Todo: change this

  // Create data packet
  ApplicationPacket p(current_term, sizeOfData);

  // Broadcast the packet
  BroadcastPacket(&p);

  // Start retransmissions for this cycle.
  RetransmitData(current_term, 1);

  // Todo: change interval to a variable
  Simulator::Schedule(Seconds(50), &Central::DisseminateData, this); 
}

/**
 * Within an interval (for one dataset size), keep retransmitting until all followers
 * get it, or max increment hit.
 *
 */
void Central::RetransmitData(int term, int increment_count){
  if (increment_count >= 20) { // Todo: implement this as a max increment
    return;
  }

  bool requireRetransmission = false;

  
  for (size_t i = 0; i < allNodes.size(); ++i) {
    bool inGroup = allNodes[i].first;
    int latestTermReplied = allNodes[i].second;

    // If node is in group, check last term
    if (inGroup && (latestTermReplied < current_term)){ // Node is in current group
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
  for (unsigned int i=0; i<= peers.size(); i++){
    if (peers[i]==ip) {
      return i;
    }
  }
  return -1;
}

/**
 * Send data packet of size sizeOfData to a specific follower node.
 */
void Central::SendData(float sizeOfData, Ipv4Address destAddr){
  // Create data packet p of size sizeOfData, send to destAddr
  ApplicationPacket p(current_term, sizeOfData);
  SendPacket(p, destAddr, false);
}


void Central::ProcessFollowerResponse(ApplicationPacket& p, Ipv4Address ip){
  int term = p.GetTerm();
  int followerId = GetIdFromIp(ip);
  
  // Record that this follower replied for this term.
  allNodes[i].second = term;
}

// Todo: some callback function for mobility/group discovery model to update nodes in group
// Group is { [node id, ip address]... }
void Central::ReceiveNewTopology(vector<pair<int, Ipv4Address>> new_group) {
  // Reset the status of allNodes
  for (size_t i = 0; i < allNodes.size(); ++i) {
    allNodes[i].first = false; // Set all to not be in current group
  }

  // Then add those in the new group
  for (&pair : new_group) {
    int id = pair.first;
    allNodes[i].first = true;
  }

}	


