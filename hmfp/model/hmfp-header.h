#ifndef HMFPHEADER_H
#define HMFPHEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"

namespace ns3 {
namespace hmfp {


enum MessageType {
    HELLO_MESSAGE = 1,
    REQUEST_MESSAGE = 2,
    REPLY_MESSAGE = 3,
    DISCONNECT_MESSAGE = 4,
    NOTIFY_MESSAGE = 5
};


class TypeHeader : public Header
{
public:
    /// c-tor
    TypeHeader (MessageType t = HELLO_MESSAGE);

    // Header serialization/deserialization
    static TypeId GetTypeId ();
    TypeId GetInstanceTypeId () const;
    uint32_t GetSerializedSize () const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);
    void Print (std::ostream &os) const;

    /// Return type
    MessageType Get () const { return m_type; }
    /// Check that type if valid
    bool IsValid () const { return m_valid; }
    bool operator== (TypeHeader const & o) const;
private:
    MessageType m_type;
    bool m_valid;
};

std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

//    Заголовок HELLO сообщения
//
//       0                   1                   2                   3
//       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |     Type      |     Reserved  |      Rtable Size              |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                    Destination Address                        |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |     Hop Count |   Reserved    |     Additional Info           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                              ...                              |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                    Destination Address                        |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |     Hop Count |   Reserved    |     Additional Info           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

struct RoutingInf
{
    Ipv4Address address;
    uint8_t hopCount;
    uint8_t reserved;
    uint16_t addInfo;
};

class HelloHeader : public Header
{
public:
    HelloHeader();
    virtual ~HelloHeader() {}

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    std::vector<RoutingInf> getRtable() { return m_rtable; }
    void setRtable(std::vector<RoutingInf> rtable) { m_rtable = rtable; m_rtableSize = m_rtable.size(); }

private:
    std::vector<RoutingInf> m_rtable;
    uint8_t m_reserved;
    uint16_t m_rtableSize;
};

//    Заголовок Request/Reply/Disconnect сообщения
//
//       0                   1                   2                   3
//       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |     Type      |     Reserved  |      Additional Info          |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

class InfoHeader : public Header
{

public:
    //    InfoHeader();
    InfoHeader(MessageType type = REQUEST_MESSAGE);
    virtual ~InfoHeader() {}

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);


private:
    MessageType m_messageType;
    uint8_t m_reserved;
    uint16_t m_addInfo;
};


//    Заголовок Notify сообщения
//
//       0                   1                   2                   3
//       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |     Type      |     Reserved  |      Additional Info          |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                    Disconnect Address                        |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

class NotifyHeader : public Header
{

public:
    NotifyHeader(Ipv4Address address = Ipv4Address ());
    virtual ~NotifyHeader() {}

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

    void SetDisconnectAddress (Ipv4Address address) { this->m_disconnectAddress = address; };

private:
    uint8_t m_reserved;
    uint16_t m_addInfo;
    Ipv4Address m_disconnectAddress;
};

}
}

#endif // HMFPHEADER_H
