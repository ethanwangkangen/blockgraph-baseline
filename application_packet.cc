#include "application_packet.h"
#include <cstring>

/**
 * Blank constructor?
 */
ApplicationPacket::ApplicationPacket(){
  service = NULL;
  payload = "";

  size = CalculateSize();
}

/**
 * Construct ApplicationPacket::Data
 * that leaders will send followers.
 * payload structure is [4 bytes: term][rest of data...]
 */
ApplicationPacket::ApplicationPacket(int term, float dataSize) {
  service = DATA;
  
  // Start with the 4-byte term
  payload.resize(sizeof(int));
  memcpy(payload.data(), &term, sizeof(int));

  // Pad with random bytes
  for (float i = 0; i < dataSize; ++i) {
    char randomByte = rand() % 256;
    payload += randomByte;
  }
  size = CalculateSize();

}

/**
 * Constructor for reply packet. 
 * Payload structure is [4 bytes term]
 */
ApplicationPacket::ApplicationPacket(int term) {
  service = REPLY;
  payload.resize(sizeof(int)); // Just need 4 bytes for the term
  memcpy(payload.data(), &term, sizeof(int));
  size = CalculateSize();
}

/**
 * Copy constructor.
 */
ApplicationPacket::ApplicationPacket(const ApplicationPacket &p){
  service = p.service;
  payload = p.payload;
  size = p.size;
}

/**
 * Constructs ApplicationPacket from serialised form.
 */
ApplicationPacket::ApplicationPacket(string &serie){
  packet_hdr_serie *hdr = (packet_hdr_serie*) serie.data();
  service = hdr->service;
  size = hdr->size;

  const char* packet_payload = serie.data() + HeaderSize();
  payload = string(packet_payload, size-HeaderSize());
}

ApplicationPacket::~ApplicationPacket(){
}

int ApplicationPacket::GetTerm() {
  if (payload.size() < sizeof(int)) {
    return -1;
  }

  int term;

  // Regardless of DATA or REPLY packet type, term is always the
  // first 4 bytes of the payload anyway.
  memcpy(&term, payload.data(), sizeof(int));
  return term;
}

/**
 * Calculate the size of the SERIALIZED version of the packet
 */
int ApplicationPacket::CalculateSize(){
  return HeaderSize() + payload.size();
}

/**
 * Header contains: 
 * int size
 * char service
 */
int ApplicationPacket::HeaderSize(){
  return sizeof(char) + sizeof(int);
}

void ApplicationPacket::SetSize(int size){
  this->size = size;
}

int ApplicationPacket::GetSize(){
  return size;
}

void ApplicationPacket::SetService(char service){
  this->service = service;
}

char ApplicationPacket::GetService(){
  return service;
}

void ApplicationPacket::SetPayload(string payload){
  this->payload = payload;
}

string ApplicationPacket::GetPayload(){
  return payload;
}


string ApplicationPacket::Serialize(){
  string ret(HeaderSize(), 0);
  packet_hdr_serie* p = (packet_hdr_serie*) ret.data();

  p->service = service;
  p->size = size;
  
  ret = ret + payload;
  return ret;
}

ostream& operator<<(ostream& o, const ApplicationPacket& p){
  o << "Packet(" << (int)p.service << "," << p.size << "," << p.payload << ")";
  return o;
}

