/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Morteza Kheirkhah <m.kheirkhah@sussex.ac.uk>
 */

// Network topology
//
//       n0 ----------- n1
// - Flow from n0 to n1 using MpTcpBulkSendApplication.

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MpTcpBulkSendExample");

int
main(int argc, char *argv[])
{
  LogComponentEnable("MpTcpSocketBase", LOG_ALL);

  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(8)); // Sink
  Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("RTT_Compensator"));
  Config::SetDefault("ns3::MpTcpSocketBase::PathManagement", StringValue("NdiffPorts"));

  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper pointToPoint1;
  pointToPoint1.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  pointToPoint1.SetChannelAttribute("Delay", StringValue("100ms"));

  PointToPointHelper pointToPoint2;
  pointToPoint2.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint2.SetChannelAttribute("Delay", StringValue("100ms"));

  NetDeviceContainer devices1;
  devices1 = pointToPoint1.Install(nodes);
  NetDeviceContainer devices2;
  devices2 = pointToPoint2.Install(nodes);

  InternetStackHelper internet;
  internet.Install(nodes);
  
  vector<Ipv4InterfaceContainer> ipv4Ints;
  Ipv4AddressHelper ipv41;
  ipv41.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4AddressHelper ipv42;
  ipv42.SetBase("10.1.2.0", "255.255.255.0");

  Ipv4InterfaceContainer interface1 = ipv41.Assign(devices1);
  Ipv4InterfaceContainer interface2 = ipv42.Assign(devices2);
  ipv4Ints.insert(ipv4Ints.end(), interface1);
  ipv4Ints.insert(ipv4Ints.end(), interface2);

  uint16_t port = 9;
  MpTcpPacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApps = sink.Install(nodes.Get(1));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(10.0));

  MpTcpBulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(interface1.GetAddress(1)), port));
  source.SetAttribute("MaxBytes", UintegerValue(100));
  ApplicationContainer sourceApps = source.Install(nodes.Get(0));
  sourceApps.Start(Seconds(0.0));
  sourceApps.Stop(Seconds(10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(Seconds(20.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO ("Done.");

}
