/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hmfp-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HmfpRoutingProtocol");

namespace hmfp {

RoutingProtocol::RoutingProtocol(): m_routingTable(new RoutingTable()) {
}

RoutingProtocol::~RoutingProtocol() {
  delete m_routingTable;
}

Ptr<Ipv4Route> RoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  return NULL;
}

bool RoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                 LocalDeliverCallback lcb, ErrorCallback ecb)
{
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
  ;
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

