/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hmfp-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HmfpRoutingProtocol");

namespace hmfp {

RoutingProtocol::RoutingProtocol(): m_routingTable() {
}

RoutingProtocol::~RoutingProtocol() {
}

Ptr<Ipv4Route> RoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
    NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));
    if (!p)
      {
        NS_LOG_DEBUG("Packet is == 0");
        return Ptr<Ipv4Route>();
      }
    sockerr = Socket::ERROR_NOTERROR;
    Ptr<Ipv4Route> route;
    Ipv4Address dst = header.GetDestination ();
    RoutingTableEntry rt;
    if (m_routingTable.LookupRoute(dst, rt))
      {
        route = rt.GetRoute ();
        NS_ASSERT (route != 0);
        NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetSource ());
        if (oif != 0 && route->GetOutputDevice () != oif)
          {
            NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
            sockerr = Socket::ERROR_NOROUTETOHOST;
            return Ptr<Ipv4Route> ();
          }
        return route;
      }
    return Ptr<Ipv4Route> ();
}

bool RoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                 LocalDeliverCallback lcb, ErrorCallback ecb)
{
    NS_LOG_FUNCTION (this << " " << m_ipv4->GetObject<Node> ()->GetId () << " " << header.GetDestination ());

    Ipv4Address dst = header.GetDestination ();
    Ipv4Address origin = header.GetSource ();

    // Consume self-originated packets
    if (IsMyOwnAddress (origin) == true) {
        return true;
    }

    // Local delivery
    NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
    uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);
    if (m_ipv4->IsDestinationAddress (dst, iif)) {
        if (!lcb.IsNull ()) {
            NS_LOG_LOGIC ("Local delivery to " << dst);
            lcb (p, header, iif);
            return true;
        } else {
            // The local delivery callback is null.  This may be a multicast
            // or broadcast packet, so return false so that another
            // multicast routing protocol can handle it.  It should be possible
            // to extend this to explicitly check whether it is a unicast
            // packet, and invoke the error callback if so
            return false;
        }
    }

    // Forwarding
    RoutingTableEntry toDst;
    if (m_routingTable.LookupRoute (dst, toDst)) {
        Ptr<Ipv4Route> route = toDst.GetRoute ();
        NS_LOG_LOGIC (route->GetSource ()<<" forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());

        ucb (route, p, header);
        return true;
    }
    return false;
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
  NS_LOG_FUNCTION (this << src);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (src == iface.GetLocal ())
        {
          return true;
        }
    }
  return false;
}

void RoutingProtocol::DoDispose() {

}

void RoutingProtocol::DoInitialize() {

}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::hmfp::RoutingProtocol")
      .SetParent<Ipv4RoutingProtocol> ()
      .SetGroupName ("Hmfp")
      .AddConstructor<RoutingProtocol> ()
      .AddAttribute("SnrBottomBound", "Нижняя граница качества сигнала (отношение сигнал/шум)",
                    DoubleValue (10.0),
                    MakeDoubleAccessor (&RoutingProtocol::m_snrBottomBound),
                    MakeDoubleChecker<double> ());
  return tid;
}

void RoutingProtocol::NotifyInterfaceUp (uint32_t interface) {
  ;
}

void RoutingProtocol::NotifyInterfaceDown (uint32_t interface) {
  ;
}

void RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  ;
}

void RoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) {
  ;
}

void RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4) {
    NS_ASSERT (ipv4 != 0);
    NS_ASSERT (m_ipv4 == 0);

    m_ipv4 = ipv4;

    // Create lo route. It is asserted that the only one interface up for now is loopback
    NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
    Ptr<NetDevice> lo = m_ipv4->GetNetDevice (0);
    NS_ASSERT (lo != 0);
    // Remember lo route
    RoutingTableEntry rt (/*device=*/ lo, /*dst=*/ Ipv4Address::GetLoopback (),
                                      /*iface=*/ Ipv4InterfaceAddress (Ipv4Address::GetLoopback (), Ipv4Mask ("255.0.0.0")),
                                      /*hops=*/ 1, /*next hop=*/ Ipv4Address::GetLoopback ());
    m_routingTable.AddRoute (rt);

    Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

void RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const {
  ;
}

void RoutingProtocol::Start() {

}

void RoutingProtocol::Recv(Ptr<Socket> socket) {

}

void RoutingProtocol::RecvRequest(Ptr<Packet> p, Ptr<Ipv4Address> from) {

}

void RoutingProtocol::RecvReply(Ptr<Packet> p, Ptr<Ipv4Address> from) {

}

void RoutingProtocol::RecvDisconnectNotification() {

}


void RoutingProtocol::SendHello() {

}

void RoutingProtocol::SendRequest() {

}

void RoutingProtocol::SendReply() {

}

void RoutingProtocol::SendDisconnectNotification() {

}

}
}

