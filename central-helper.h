#ifndef CENTRAL_HELPER_H
#define CENTRAL_HELPER_H

#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

#include "b4m_traces.h"
#include "Central.h"
#include <vector>

using namespace std;
using namespace ns3;
class CentralHelper{
  public:
    CentralHelper();
    CentralHelper(B4MTraces* t);
    ~CentralHelper();

    ApplicationContainer Install(NodeContainer c, float timeBetweenTxn);

  private:
    ObjectFactory factory;
    B4MTraces* traces;
};

#endif
