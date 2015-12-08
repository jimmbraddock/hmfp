/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "hmfp-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/node.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "hmfp-header.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HmfpRoutingProtocol");

namespace hmfp {

NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

RoutingProtocol::RoutingProtocol(): m_ipv4 (0), m_htimer (Timer::CANCEL_ON_DESTROY), m_routingTable() {
    m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
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
    m_ipv4 = 0;

    for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddresses.begin ();
         iter != m_socketAddresses.end (); iter++)
      {
        iter->first->Close ();
      }
    m_socketAddresses.clear ();

    Ipv4RoutingProtocol::DoDispose ();
}

void RoutingProtocol::DoInitialize() {
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("OLSR on node " << m_ipv4->GetObject<Node> ()->GetId () << " started");
    HelloTimerExpire();
    Ipv4RoutingProtocol::DoInitialize ();
}

void RoutingProtocol::HelloTimerExpire() {
    SendHello ();
    m_htimer.Schedule (m_helloInterval);
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::hmfp::RoutingProtocol")
      .SetParent<Ipv4RoutingProtocol> ()
      .SetGroupName ("Hmfp")
      .AddConstructor<RoutingProtocol> ()
      .AddAttribute ("HelloInterval", "HELLO messages emission interval.",
                     TimeValue (Seconds (2)),
                     MakeTimeAccessor (&RoutingProtocol::m_helloInterval),
                     MakeTimeChecker ())
      .AddAttribute ("SnrBottomBound", "Нижняя граница качества сигнала (отношение сигнал/шум)",
                     DoubleValue (10.0),
                     MakeDoubleAccessor (&RoutingProtocol::m_snrBottomBound),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("UniformRv",
                     "Access to the underlying UniformRandomVariable",
                     StringValue ("ns3::UniformRandomVariable"),
                     MakePointerAccessor (&RoutingProtocol::m_uniformRandomVariable),
                     MakePointerChecker<UniformRandomVariable> ());
  return tid;
}

void RoutingProtocol::NotifyInterfaceUp (uint32_t interface) {
    NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());
    Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
    if (l3->GetNAddresses (interface) > 1)
      {
        NS_LOG_WARN ("HMFP does not work with more then one address per each interface.");
      }
    Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
    if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
      return;

    // Create a socket to listen only on this interface
    Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                               UdpSocketFactory::GetTypeId ());
    NS_ASSERT (socket != 0);
    socket->SetRecvCallback (MakeCallback (&RoutingProtocol::Recv, this));
    socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), HMFP_PORT));
    socket->BindToNetDevice (l3->GetNetDevice (interface));
    socket->SetAllowBroadcast (true);
    socket->SetAttribute ("IpTtl", UintegerValue (1));
    m_socketAddresses.insert (std::make_pair (socket, iface));


    // Add local broadcast record to the routing table
    Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
    RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*iface=*/ iface,
                                      /*hops=*/ 1, /*next hop=*/ iface.GetBroadcast ());
    m_routingTable.AddRoute (rt);
}

void RoutingProtocol::NotifyInterfaceDown (uint32_t interface) {
    NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());

    // Close socket
    Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (interface, 0));
    NS_ASSERT (socket);
    socket->Close ();
    m_socketAddresses.erase (socket);

    if (m_socketAddresses.empty ())
      {
        NS_LOG_LOGIC ("No hmfp interfaces");
        m_htimer.Cancel ();
        m_routingTable.Clear ();
        return;
      }
    m_routingTable.DeleteAllRoutesFromInterface (m_ipv4->GetAddress (interface, 0));
}

Ptr<Socket>
RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
  NS_LOG_FUNCTION (this << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      if (iface == addr)
        return socket;
    }
  Ptr<Socket> socket;
  return socket;
}

void RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) {
    NS_LOG_FUNCTION (this << " interface " << interface << " address " << address);
    Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
    if (!l3->IsUp (interface))
      return;
    if (l3->GetNAddresses (interface) == 1)
      {
        Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
        Ptr<Socket> socket = FindSocketWithInterfaceAddress (iface);
        if (!socket)
          {
            if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
              return;
            // Create a socket to listen only on this interface
            Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                       UdpSocketFactory::GetTypeId ());
            NS_ASSERT (socket != 0);
            socket->SetRecvCallback (MakeCallback (&RoutingProtocol::Recv,this));
            socket->Bind (InetSocketAddress (iface.GetLocal (), HMFP_PORT));
            socket->BindToNetDevice (l3->GetNetDevice (interface));
            socket->SetAllowBroadcast (true);
            m_socketAddresses.insert (std::make_pair (socket, iface));

            // Add local broadcast record to the routing table
            Ptr<NetDevice> dev = m_ipv4->GetNetDevice (
                m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
            RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*iface=*/ iface, /*hops=*/ 1,
                                              /*next hop=*/ iface.GetBroadcast ());
            m_routingTable.AddRoute (rt);
          }
      }
    else
      {
        NS_LOG_LOGIC ("HMFP does not work with more then one address per each interface. Ignore added address");
      }
}

void RoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) {
    NS_LOG_FUNCTION (this);
    Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
    if (socket)
      {
        m_routingTable.DeleteAllRoutesFromInterface (address);
        socket->Close ();
        m_socketAddresses.erase (socket);

        Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
        if (l3->GetNAddresses (interface))
          {
            Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
            // Create a socket to listen only on this interface
            Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                       UdpSocketFactory::GetTypeId ());
            NS_ASSERT (socket != 0);
            socket->SetRecvCallback (MakeCallback (&RoutingProtocol::Recv, this));
            // Bind to any IP address so that broadcasts can be received
            socket->Bind (InetSocketAddress (iface.GetLocal (), HMFP_PORT));
            socket->BindToNetDevice (l3->GetNetDevice (interface));
            socket->SetAllowBroadcast (true);
            socket->SetAttribute ("IpTtl", UintegerValue (1));
            m_socketAddresses.insert (std::make_pair (socket, iface));

            // Add local broadcast record to the routing table
            Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
            RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*iface=*/ iface,
                                              /*hops=*/ 1, /*next hop=*/ iface.GetBroadcast ());
            m_routingTable.AddRoute (rt);
          }
        if (m_socketAddresses.empty ())
          {
            NS_LOG_LOGIC ("No hmfp interfaces");
            m_htimer.Cancel ();
            m_routingTable.Clear ();
            return;
          }
      }
    else
      {
        NS_LOG_LOGIC ("Remove address not participating in HMFP operation");
      }
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

    m_htimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
}

void RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const {
  ;
}

void RoutingProtocol::Recv(Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
    Address sourceAddress;
    Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
    InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
    Ipv4Address sender = inetSourceAddr.GetIpv4 ();
    Ipv4Address receiver;

    // TODO: Как в таком случае принимается HELLO?
    if (m_socketAddresses.find (socket) != m_socketAddresses.end ()) {
        receiver = m_socketAddresses[socket].GetLocal ();
    } else {
        NS_ASSERT_MSG (false, "Received a packet from an unknown socket");
    }
    NS_LOG_DEBUG ("HMFP node " << this << " received a HMFP packet from " << sender << " to " << receiver);

    // Обрабатываем только тип заголовка. Остальное полезное в заголовке обработаем уже позже более конкретно
    TypeHeader tHeader (HELLO_MESSAGE);
    packet->RemoveHeader (tHeader);
    if (!tHeader.IsValid ())
    {
        NS_LOG_DEBUG ("HMFP message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
        return; // drop
    }

    switch (tHeader.Get ())
    {
    case HELLO_MESSAGE:
    {
        RecvHello (packet, receiver, sender);
        break;
    }
    case REQUEST_MESSAGE:
    case REPLY_MESSAGE:
    case DISCONNECT_MESSAGE:
    {
        RecvInfoMessage(packet, receiver, sender);
        break;
    }
    case NOTIFY_MESSAGE:
    {
        RecvNotify (packet, receiver, sender);
        break;
    }
    }
}

void RoutingProtocol::RecvHello(Ptr<Packet> p, Ipv4Address to, Ipv4Address from) {
    NS_LOG_FUNCTION (this << " src " << from);
    HelloHeader helloHeader;
    p->RemoveHeader (helloHeader);

    // Если узел новый, то добавим его в таблицу маршрутизации
    RoutingTableEntry neighbor;
    if (!m_routingTable.LookupRoute(from, neighbor)) {
        Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (to));
        RoutingTableEntry newEntry (/*device=*/ dev, /*dst=*/ from,
                                                /*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (to), 0),
                                                /*hop=*/ 1, /*nextHop=*/ from);
        m_routingTable.AddRoute (newEntry);
    }


    NS_LOG_FUNCTION (this << "from " << from);
    for (std::vector<RoutingInf>::const_iterator it = helloHeader.getRtable().begin(); it != helloHeader.getRtable().end(); ++it) {
        RoutingTableEntry existPath;

        // Новый маршрут сразу добавим в таблицу маршрутизации, а для существующих проверим количество переходов
        bool isNotNeedCreateNew = m_routingTable.LookupRoute(it->address, existPath);
        if (isNotNeedCreateNew && existPath.GetHop() > it->hopCount + 1) {
            m_routingTable.DeleteRoute(it->address);
            isNotNeedCreateNew = false;
        }
        if (!isNotNeedCreateNew) {
            Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (it->address));
            RoutingTableEntry newEntry (/*device=*/ dev, /*dst=*/ it->address,
                                                    /*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (it->address), 0),
                                                    /*hop=*/ it->hopCount + 1, /*nextHop=*/ from);
            m_routingTable.AddRoute (newEntry);
        }

    }
}

void RoutingProtocol::RecvInfoMessage(Ptr<Packet> p, Ipv4Address to, Ipv4Address from) {

}

void RoutingProtocol::RecvNotify(Ptr<Packet> p, Ipv4Address to, Ipv4Address from) {

}



void RoutingProtocol::SendHello() {

}

void RoutingProtocol::SendRequest() {

}

void RoutingProtocol::SendReply() {

}

void RoutingProtocol::SendDisconnectNotification() {

}

int64_t
RoutingProtocol::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}

}
}

