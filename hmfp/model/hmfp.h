/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef HMFP_H
#define HMFP_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/timer.h"

namespace ns3 {
namespace hmfp {

class RoutingProtocol : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId (void);

  RoutingProtocol ();
//protected:
//  virtual void DoInitialize (void) {};
private:
  virtual ~RoutingProtocol () {};
  // From Ipv4RoutingProtocol
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet>,
                                      const Ipv4Header &,
                                      Ptr<NetDevice>,
                                      Socket::SocketErrno) {return NULL;}

  virtual bool RouteInput (Ptr<const Packet> ,
                           const Ipv4Header &,
                           Ptr<const NetDevice> ,
                           UnicastForwardCallback ,
                           MulticastForwardCallback ,
                           LocalDeliverCallback,
                           ErrorCallback) {return false;}

  virtual void NotifyInterfaceUp (uint32_t interface) {}
  virtual void NotifyInterfaceDown (uint32_t interface) {}
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address) {}
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address) {}
  virtual void SetIpv4 (Ptr<Ipv4> ipv4) {}
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const {}

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




  Time m_param;

};

}
}

#endif /* HMFP_H */

