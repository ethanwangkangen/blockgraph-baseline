#include "central-helper.h"

CentralHelper::CentralHelper(){
  factory.SetTypeId("Central");
}

CentralHelper::CentralHelper(B4MTraces* t){
  factory.SetTypeId("Central");
  traces=t;
}

CentralHelper::~CentralHelper(){
}


ApplicationContainer CentralHelper::Install(NodeContainer c, float timeBetweenTxn){
  ApplicationContainer apps;

  // Get list of ip addresses
  vector<Ipv4Address> peers;
  for (uint32_t i=0; i<c.GetN(); ++i){
    Ptr<Ipv4> ipv4 = c.Get(i)->GetObject<Ipv4>();
    peers.push_back(ipv4->GetAddress(1, 0).GetLocal());
  }

  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i){
    cout << "Install Central on node : " << (*i)->GetId() << endl;
    Ptr<Central> CentralApp = factory.Create<Central>();
    CentralApp->SetUp(*i, peers); // Pass in current node, list of all peer IP, and timeBetweenTxn value
    // CentralApp->traces = traces;
    (*i)->AddApplication(CentralApp);
    apps.Add(CentralApp);
  }
  return apps;
}
