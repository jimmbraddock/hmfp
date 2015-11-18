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

void HelloHeader::Print (std::ostream &os) const {

}

uint32_t HelloHeader::Deserialize (Buffer::Iterator start) {
    return 0;
}
}
}
