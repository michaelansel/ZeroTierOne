/*
 * ZeroTier One - Network Virtualization Everywhere
 * Copyright (C) 2011-2016  ZeroTier, Inc.  https://www.zerotier.com/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Constants.hpp"
#include "RuntimeEnvironment.hpp"
#include "OutboundMulticast.hpp"
#include "Switch.hpp"
#include "Network.hpp"
#include "Node.hpp"
#include "Peer.hpp"
#include "Topology.hpp"

namespace ZeroTier {

void OutboundMulticast::init(
	const RuntimeEnvironment *RR,
	uint64_t timestamp,
	uint64_t nwid,
	unsigned int limit,
	unsigned int gatherLimit,
	const MAC &src,
	const MulticastGroup &dest,
	unsigned int etherType,
	const void *payload,
	unsigned int len)
{
	_timestamp = timestamp;
	_nwid = nwid;
	_limit = limit;

	uint8_t flags = 0;
	if (gatherLimit) flags |= 0x02;
	if (src) flags |= 0x04;

	/*
	TRACE(">>MC %.16llx INIT %.16llx/%s limit %u gatherLimit %u from %s to %s length %u",
		(unsigned long long)this,
		nwid,
		dest.toString().c_str(),
		limit,
		gatherLimit,
		(src) ? src.toString().c_str() : MAC(RR->identity.address(),nwid).toString().c_str(),
		dest.toString().c_str(),
		len);
	*/

	_packet.setSource(RR->identity.address());
	_packet.setVerb(Packet::VERB_MULTICAST_FRAME);
	_packet.append((uint64_t)nwid);
	_packet.append(flags);
	if (gatherLimit) _packet.append((uint32_t)gatherLimit);
	if (src) src.appendTo(_packet);
	dest.mac().appendTo(_packet);
	_packet.append((uint32_t)dest.adi());
	_packet.append((uint16_t)etherType);
	_packet.append(payload,len);
	_packet.compress();
}

void OutboundMulticast::sendOnly(const RuntimeEnvironment *RR,const Address &toAddr)
{
	// TODO: apply Filter

	SharedPtr<Peer> peer(RR->topology->getPeer(toAddr));
	if (peer) {
		// TODO: push creds if needed
	}

	//TRACE(">>MC %.16llx -> %s",(unsigned long long)this,toAddr.toString().c_str());
	_packet.newInitializationVector();
	_packet.setDestination(toAddr);
	RR->sw->send(_packet,true,_nwid);
}

} // namespace ZeroTier
