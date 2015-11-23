#ifndef HMFPRTABLE
#define HMFPRTABLE

#include <stdint.h>
#include <cassert>
#include <map>
#include <sys/types.h>
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3 {
namespace hmfp {


/**
 * \ingroup hmfp
 * \brief Routing table entry
 */
class RoutingTableEntry
{
public:
  /// c-to
  RoutingTableEntry (Ptr<NetDevice> dev = 0,Ipv4Address dst = Ipv4Address (),
                     Ipv4InterfaceAddress iface = Ipv4InterfaceAddress (), uint16_t  hops = 0,
                     Ipv4Address nextHop = Ipv4Address ());
  RoutingTableEntry () {}

  ~RoutingTableEntry () {}


  // Fields
  Ipv4Address GetDestination () const { return m_ipv4Route->GetDestination (); }
  Ptr<Ipv4Route> GetRoute () const { return m_ipv4Route; }
  void SetRoute (Ptr<Ipv4Route> r) { m_ipv4Route = r; }
  void SetNextHop (Ipv4Address nextHop) { m_ipv4Route->SetGateway (nextHop); }
  Ipv4Address GetNextHop () const { return m_ipv4Route->GetGateway (); }
  void SetOutputDevice (Ptr<NetDevice> dev) { m_ipv4Route->SetOutputDevice (dev); }
  Ptr<NetDevice> GetOutputDevice () const { return m_ipv4Route->GetOutputDevice (); }
  Ipv4InterfaceAddress GetInterface () const { return m_iface; }
  void SetInterface (Ipv4InterfaceAddress iface) { m_iface = iface; }

  void SetHop (uint16_t hop) { m_hops = hop; }
  uint16_t GetHop () const { return m_hops; }

  /**
   * \brief Compare destination address
   * \return true if equal
   */
  bool operator== (Ipv4Address const  dst) const
  {
    return (m_ipv4Route->GetDestination () == dst);
  }
  void Print (Ptr<OutputStreamWrapper> stream) const;

private:
  /// Hop Count (number of hops needed to reach destination)
  uint16_t m_hops;

  Ptr<Ipv4Route> m_ipv4Route;
  /// Output interface address
  Ipv4InterfaceAddress m_iface;
};


//================================================================================================================
/**
 * \ingroup hmfp
 * \brief The Routing table used by HMFP protocol
 */
class RoutingTable
{
public:
  /// c-tor
  RoutingTable () {}

  /**
   * Add routing table entry if it doesn't yet exist in routing table
   * \param r routing table entry
   * \return true in success
   */
  bool AddRoute (RoutingTableEntry & r);

  /**
   * Delete routing table entry with destination address dst, if it exists.
   * \param dst destination address
   * \return true on success
   */
  bool DeleteRoute (Ipv4Address dst);

  /**
   * Lookup routing table entry with destination address dst
   * \param dst destination address
   * \param rt entry with destination address dst, if exists
   * \return true on success
   */
  bool LookupRoute (Ipv4Address dst, RoutingTableEntry & rt);

  /// Update routing table
  bool Update (RoutingTableEntry & rt);

  /// Delete all route from interface with address iface
  void DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface);

  /// Delete all entries from routing table
  void Clear () { m_ipv4AddressEntry.clear (); }

  /// Print routing table
  void Print (Ptr<OutputStreamWrapper> stream) const;

private:
  std::map<Ipv4Address, RoutingTableEntry> m_ipv4AddressEntry;
};

}
}



#endif // HMFPRTABLE

