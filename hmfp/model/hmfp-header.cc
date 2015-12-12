#include "hmfp-header.h"

namespace ns3 {

//NS_LOG_COMPONENT_DEFINE ("HmfpRoutingProtocol");

namespace hmfp {

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t) :
  m_type (t), m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::hmfp::TypeHeader")
    .SetParent<Header> ()
    .SetGroupName("Hmfp")
    .AddConstructor<TypeHeader> ()
  ;
  return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
  return 1;
}

void
TypeHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  m_valid = true;
  switch (type)
  {
  case HELLO_MESSAGE:
  case REQUEST_MESSAGE:
  case REPLY_MESSAGE:
  case DISCONNECT_MESSAGE:
  case NOTIFY_MESSAGE:
  {
      m_type = (MessageType) type;
      break;
  }
  default:
      m_valid = false;
  }
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
TypeHeader::Print (std::ostream &os) const
{
    switch (m_type)
    {
    case HELLO_MESSAGE:
    {
        os << "HELLO";
        break;
    }
    case REQUEST_MESSAGE:
    {
        os << "REQUEST";
        break;
    }
    case REPLY_MESSAGE:
    {
        os << "REPLY";
        break;
    }
    case DISCONNECT_MESSAGE:
    {
        os << "DISCONNECT";
        break;
    }
    case NOTIFY_MESSAGE:
    {
        os << "NOTIFY";
        break;
    }
    default:
        os << "UNKNOWN_TYPE";
    }
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
  return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}

//=====================================================================================================================
//                                 Hello
//====================================================================================================================

HelloHeader::HelloHeader () :
  m_rtable (0), m_reserved(0), m_rtableSize(0)
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
    return 3 + 8 * m_rtableSize;
}

void HelloHeader::Serialize (Buffer::Iterator start) const {
    Buffer::Iterator i = start;

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
    return 3;
}

void InfoHeader::Serialize (Buffer::Iterator start) const {
    Buffer::Iterator i = start;

    i.WriteU8 (0); // Reserved
    i.WriteU16 (m_addInfo);
}

uint32_t InfoHeader::Deserialize (Buffer::Iterator start) {
    Buffer::Iterator i = start;
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
  m_reserved(0), m_addInfo(0), m_disconnectAddress(address)
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
    return 7;
}

void NotifyHeader::Serialize (Buffer::Iterator start) const {
    Buffer::Iterator i = start;

    i.WriteU8 (0); // Reserved
    i.WriteU16 (this->m_addInfo);
    WriteTo(i, m_disconnectAddress);
}

uint32_t NotifyHeader::Deserialize (Buffer::Iterator start) {
    Buffer::Iterator i = start;
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
