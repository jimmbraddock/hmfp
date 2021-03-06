/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hmfp-helper.h"
#include "ns3/hmfp-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{

HmfpHelper::HmfpHelper() :
  Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::hmfp::RoutingProtocol");
}

HmfpHelper*
HmfpHelper::Copy (void) const
{
  return new HmfpHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
HmfpHelper::Create (Ptr<Node> node) const
{
  Ptr<hmfp::RoutingProtocol> agent = m_agentFactory.Create<hmfp::RoutingProtocol> ();
  node->AggregateObject (agent);
  return agent;
}

void
HmfpHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

int64_t
HmfpHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<hmfp::RoutingProtocol> hmfp = DynamicCast<hmfp::RoutingProtocol> (proto);
      if (hmfp)
        {
          currentStream += hmfp->AssignStreams (currentStream);
          continue;
        }
      // hmfp may also be in a list
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv4RoutingProtocol> listProto;
          Ptr<hmfp::RoutingProtocol> listhmfp;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listhmfp = DynamicCast<hmfp::RoutingProtocol> (listProto);
              if (listhmfp)
                {
                  currentStream += listhmfp->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }
  return (currentStream - stream);
}


}

