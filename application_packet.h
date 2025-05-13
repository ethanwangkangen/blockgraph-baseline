#ifndef APPLICATION_PACKET_H
#define APPLICATION_PACKET_H
#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

class ApplicationPacket{
  public:
    
    // Header, used for serialised version of packet
    typedef struct packet_hdr_serie{
      int size; 
      char service;
    } packet_hdr_serie;

  public:
    // Constants
    enum {DATA, REPLY};
  public:
    //Constructors and destructor
    ApplicationPacket();
    //ApplicationPacket(char service, string payload);
    ApplicationPacket(int term, float dataSize); // Create data packet
    APplicationPacket(int term); // Create reply packet
    ApplicationPacket(const ApplicationPacket &p);
    ApplicationPacket(string &serie);
    ~ApplicationPacket();

  public:
    // Getters, Setters and Operators;
    
    int GetTerm();

    void SetSize(int size);
    int GetSize();
    void SetService(char service);
    char GetService();
    void SetPayload(string payload);
    string GetPayload();

    ostream& operator<<(const ostream& o);
    friend ostream& operator<<(ostream& o, const ApplicationPacket& p);
  public:
    /**
     * Calculate and return the total size of the packet
     */
    int CalculateSize();

    /**
     * Calculate and return the size of the header of the packet
     */
    int HeaderSize();

    /**
     * Serialize the packet into an array of byte (here a string is more
     * convenient)
     */
    string Serialize();

  private:
    char service; // Service identifier of the packet (DATA, REPLY}
    int size; // Size of the whole packet (INCLUDING HEADER!!!)
    string payload; // Payload of the packet (contains an application message

};

#endif

