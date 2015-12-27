/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef HMFP_H
#define HMFP_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/timer.h"
#include "hmfp-rtable.h"
#include "ns3/random-variable-stream.h"
#include "ns3/pointer.h"

namespace ns3 {
namespace hmfp {

const int32_t HMFP_PORT = 4444;

class RoutingProtocol : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId (void);

  RoutingProtocol ();
  void SetSnrBottomBound(double value) { this->m_snrBottomBound = value; }
  double GetSnrBottomBound() const { return m_snrBottomBound; }
  void DoDispose ();
  int64_t AssignStreams (int64_t stream);
protected:
  virtual void DoInitialize (void);
private:
  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);
  virtual ~RoutingProtocol ();
  // From Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb);

  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

  // Запук работы протокола
  void Start();

  void Recv(Ptr<Socket> socket);

  // Прием Hello сообщения
  void RecvHello(Ptr<Packet> p, Ipv4Address to, Ipv4Address from);

  // Прием REQUEST сообщения
  void RecvRequestMessage(Ptr<Packet> p, Ipv4Address to, Ipv4Address from);

  // Прием REPLY сообщения
  void RecvReplyMessage(Ptr<Packet> p, Ipv4Address to, Ipv4Address from);

  // Прием DISCONNECT сообщения
  void RecvDisconnectMessage(Ptr<Packet> p, Ipv4Address to, Ipv4Address from);

  // Поиск нового маршрута до удаляемого узла, пришедшего по Disconnect
  void RecvNotify(Ptr<Packet> p, Ipv4Address to, Ipv4Address from);


  // Отправка HELLO сообщения
  void SendHello();

  // Отправка запроса на получение сведений о качестве сигнала
  void SendRequest(Ptr<Socket> socket, Ipv4Address destination);

  // Отправка ответа для получения качества сигнала на приемнике
  void SendReply(Ptr<Socket> socket, Ipv4Address destination);

  // Отправка уведомления о скором разрыве соединения
  void SendDisconnectNotification(Ptr<Socket> socket, Ipv4Address destination);

  bool IsMyOwnAddress (Ipv4Address src);

  void HelloTimerExpire();

  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const;

  Ptr<Ipv4> m_ipv4;
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;

  // Hello таймер
  Timer m_htimer;
  Time m_helloInterval;

  /// Routing table
  RoutingTable m_routingTable;
  double m_snrBottomBound;

  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};

}
}

#endif /* HMFP_H */

