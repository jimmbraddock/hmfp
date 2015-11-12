/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hmfp.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HmfpRoutingProtocol");

namespace hmfp {

  TypeId
  RoutingProtocol::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::hmfp::RoutingProtocol")
      .SetParent<Ipv4RoutingProtocol> ()
      .SetGroupName ("Hmfp")
      .AddConstructor<RoutingProtocol> ()
      .AddAttribute ("PeriodicUpdateInterval","Periodic interval between exchange of full routing tables among nodes. ",
                     TimeValue (Seconds (15)),
                     MakeTimeAccessor (&RoutingProtocol::m_param),
                     MakeTimeChecker ());
    return tid;
  }


  RoutingProtocol::RoutingProtocol () {

  }



}
}

