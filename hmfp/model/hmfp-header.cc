#include "hmfp-header.h"

namespace ns3 {

//NS_LOG_COMPONENT_DEFINE ("HmfpRoutingProtocol");

namespace hmfp {

HelloHeader::HelloHeader () :
  m_rtable (0), m_reserved(0), m_messageType (HELLO_MESSAGE), m_rtableSize(0)
{
}

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::hmfp::HelloHeader")
    .SetParent<Header> ()
    .SetGroupName("hmfp")
    .AddConstructor<HelloHeader> ()
  ;
  return tid;
}

TypeId
HelloHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t HelloHeader::GetSerializedSize (void) const {
    return 4 + 8 * m_rtableSize;
}

void HelloHeader::Serialize (Buffer::Iterator start) const {
    Buffer::Iterator i = start;

    i.WriteU8 (this->m_messageType);
    i.WriteU8 (0); // Reserved
    i.WriteU16 (this->m_rtableSize);
    for (std::vector<RoutingInf>::const_iterator iter = m_rtable.begin ();
         iter != m_rtable.end (); ++iter) {
        WriteTo(i, iter->address);
        i.WriteU8(iter->hopCount);
        i.WriteU8(0);
        i.WriteU16 (iter->addInfo);
    }
}

uint32_t HelloHeader::Deserialize (Buffer::Iterator start) {
    Buffer::Iterator i = start;
    m_messageType  = (MessageType) i.ReadU8 ();
    NS_ASSERT (m_messageType == HELLO_MESSAGE);
    m_reserved = i.ReadU8 ();
    m_rtableSize = i.ReadU16 ();

    m_rtable.clear ();
    for (uint8_t k = 0; k < m_rtableSize; ++k)
    {
        RoutingInf *inf = new RoutingInf();
        ReadFrom (i, inf->address);
        inf->hopCount = i.ReadU8();
        inf->reserved = i.ReadU8();
        inf->addInfo = i.ReadU16();
        m_rtable.push_back(*inf);
    }

    uint32_t dist = i.GetDistanceFrom (start);
    NS_ASSERT (dist == GetSerializedSize ());
    return dist;
}

void HelloHeader::Print (std::ostream &os) const {
    os << "HELLO сообщение. Таблица маршрутизации (узел, количество хопов):";
    std::vector<RoutingInf>::const_iterator j;
    for (j = m_rtable.begin (); j != m_rtable.end (); ++j)
    {
        os << (*j).address << " - " << (*j).hopCount;
    }
}


// ====================================================================================================================
//                                         InfoHeader
// ===================================================================================================================

//InfoHeader::InfoHeader () : m_messageType(REQUEST_MESSAGE), m_reserved(0), m_addInfo(0)
//{
//}

InfoHeader::InfoHeader (MessageType type) :
  m_messageType(type), m_reserved(0), m_addInfo(0)
{
}

NS_OBJECT_ENSURE_REGISTERED (InfoHeader);

TypeId
InfoHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::hmfp::InfoHeader")
    .SetParent<Header> ()
    .SetGroupName("hmfp")
    .AddConstructor<InfoHeader> ();
  return tid;
}

TypeId
InfoHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t InfoHeader::GetSerializedSize (void) const {
    return 4;
}

void InfoHeader::Serialize (Buffer::Iterator start) const {
    Buffer::Iterator i = start;

    i.WriteU8 (m_messageType);
    i.WriteU8 (0); // Reserved
    i.WriteU16 (m_addInfo);
}

uint32_t InfoHeader::Deserialize (Buffer::Iterator start) {
    Buffer::Iterator i = start;
    m_messageType  = (MessageType) i.ReadU8 ();
    NS_ASSERT (m_messageType == REQUEST_MESSAGE ||
               m_messageType == REPLY_MESSAGE ||
               m_messageType == DISCONNECT_MESSAGE);
    m_reserved = i.ReadU8 ();
    m_addInfo = i.ReadU16 ();

    uint32_t dist = i.GetDistanceFrom (start);
    NS_ASSERT (dist == GetSerializedSize ());
    return dist;
}

void InfoHeader::Print (std::ostream &os) const {
    os << "INFO сообщение. " << "Тип " << m_messageType << "; "
       << " доп. информация: " << m_addInfo;
}

//=================================================================================================================
//                                       NotifyMessage
//=================================================================================================================

NotifyHeader::NotifyHeader (Ipv4Address address) :
  m_messageType (NOTIFY_MESSAGE), m_reserved(0), m_addInfo(0), m_disconnectAddress(address)
{
}

NS_OBJECT_ENSURE_REGISTERED (NotifyHeader);

TypeId
NotifyHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::hmfp::NotifyHeader")
    .SetParent<Header> ()
    .SetGroupName("hmfp")
    .AddConstructor<NotifyHeader> ()
  ;
  return tid;
}

TypeId
NotifyHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t NotifyHeader::GetSerializedSize (void) const {
    return 8;
}

void NotifyHeader::Serialize (Buffer::Iterator start) const {
    Buffer::Iterator i = start;

    i.WriteU8 (this->m_messageType);
    i.WriteU8 (0); // Reserved
    i.WriteU16 (this->m_addInfo);
    WriteTo(i, m_disconnectAddress);
}

uint32_t NotifyHeader::Deserialize (Buffer::Iterator start) {
    Buffer::Iterator i = start;
    m_messageType  = (MessageType) i.ReadU8 ();
    NS_ASSERT (m_messageType == NOTIFY_MESSAGE);
    m_reserved = i.ReadU8 ();
    m_addInfo = i.ReadU16 ();
    ReadFrom(i, m_disconnectAddress);

    uint32_t dist = i.GetDistanceFrom (start);
    NS_ASSERT (dist == GetSerializedSize ());
    return dist;
}

void NotifyHeader::Print (std::ostream &os) const {
    os << "NOTIFY сообщение. Требуется найти новый маршрут к узлу: " << m_disconnectAddress;
}

}
}
