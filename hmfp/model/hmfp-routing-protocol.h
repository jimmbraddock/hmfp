/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef HMFP_H
#define HMFP_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/timer.h"
#include "hmfp-rtable.h"

namespace ns3 {
namespace hmfp {

class RoutingProtocol : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId (void);

  RoutingProtocol ();
  void SetSnrBottomBound(double value) { this->m_snrBottomBound = value; }
  double GetSnrBottomBound() const { return m_snrBottomBound; }
  void DoDispose ();
protected:
  virtual void DoInitialize (void);
private:
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

  // Прием запроса на получение сведений о качестве сигнала
  void RecvRequest(Ptr<Packet> p, Ptr<Ipv4Address> from);

  // Прием запроса с получением данных о качестве сигнала до соседа
  void RecvReply(Ptr<Packet> p, Ptr<Ipv4Address> from);

  // Прием уведомления о скором разрыве соединения в виду слабого сигнала
  void RecvDisconnectNotification();


  // Отправка HELLO сообщения
  void SendHello();

  // Отправка запроса на получение сведений о качестве сигнала
  void SendRequest();

  // Отправка ответа для получения качества сигнала на приемнике
  void SendReply();

  // Отправка уведомления о скором разрыве соединения
  void SendDisconnectNotification();



  /// Routing table
  RoutingTable *m_routingTable;
  double m_snrBottomBound;
};

}
}

#endif /* HMFP_H */

