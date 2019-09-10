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
 */

/*
	LAB Assignment #2
    1. Create a simple dumbbell topology, two client Node1 and Node2 on
    the left side of the dumbbell and server nodes Node3 and Node4 on the
    right side of the dumbbell. Let Node5 and Node6 form the bridge of the
    dumbbell. Use point to point links.

    2. Install a TCP socket instance on Node1 that will connect to Node3.

    3. Install a UDP socket instance on Node2 that will connect to Node4.

    4. Start the TCP application at time 1s.

    5. Start the UDP application at time 20s at rate Rate1 such that it clogs
    half the dumbbell bridge's link capacity.

    6. Increase the UDP application's rate at time 30s to rate Rate2
    such that it clogs the whole of the dumbbell bridge's capacity.

    7. Use the ns-3 tracing mechanism to record changes in congestion window
    size of the TCP instance over time. Use gnuplot/matplotlib to visualise plots of cwnd vs time.

    8. Mark points of fast recovery and slow start in the graphs.

    9. Perform the above experiment for TCP variants Tahoe, Reno and New Reno,
    all of which are available with ns-3.

	Solution by: Konstantinos Katsaros (K.Katsaros@surrey.ac.uk)
	based on fifth.cc
*/

// Give option for TCP Variant
// Network Topology
//
//       n0 ---+      +--- n7
//             |      |
//       n1 ---+      +--- n8
//             |      |
//       n2 ---n5 -- n6--- n9
//             |      |
//       n3 ---+      +--- n10
//             |      |
//       n4 ---+      +--- n11
//
// - All links are P2P with 500kb/s and 2ms
// - TCP flow form n0 to n2
// - UDP flow from n1 to n3

// Network topology
//
//       n0 ---+      +--- n2
//             |      |
//             n4 -- n5
//             |      |
//       n1 ---+      +--- n3
//
// - All links are P2P with 500kb/s and 2ms
// - TCP flow form n0 to n2
// - UDP flow from n1 to n3

#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Lab2");
AsciiTraceHelper ascii;

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets,
              DataRate dataRate);
  void ChangeRate (DataRate newrate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket> m_socket;
  Address m_peer;
  uint32_t m_packetSize;
  uint32_t m_nPackets;
  DataRate m_dataRate;
  EventId m_sendEvent;
  bool m_running;
  uint32_t m_packetsSent;
};

MyApp::MyApp ()
    : m_socket (0),
      m_peer (),
      m_packetSize (0),
      m_nPackets (0),
      m_dataRate (0),
      m_sendEvent (),
      m_running (false),
      m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets,
              DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

void
MyApp::ChangeRate (DataRate newrate)
{
  m_dataRate = newrate;
  return;
}

// static void
// CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
// {

//   std::cout << Simulator::Now ().GetSeconds () << "\t" << newCwnd << "\n";
// }

void
IncRate (Ptr<MyApp> app, DataRate rate)
{
  app->ChangeRate (rate);
  return;
}

// Function to record Congestion Window values
static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
}

//Trace Congestion window length
static void
TraceCwnd (Ptr<OutputStreamWrapper> stream)
{
  //Trace changes to the congestion window
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
                                 MakeBoundCallback (&CwndChange, stream));
}

int
main (int argc, char *argv[])
{
  std::string lat = "2ms";
  std::string rate = "500kb/s"; // P2P link
  bool enableFlowMonitor = false;
  std::string prot = "TcpVegas";
  CommandLine cmd;
  cmd.AddValue ("latency", "P2P link Latency in miliseconds", lat);
  cmd.AddValue ("rate", "P2P data rate in bps", rate);
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);

  cmd.Parse (argc, argv);
  std::string c_s = "dumbell_cw_12nodes.dat";
  Ptr<OutputStreamWrapper> dumbell_cw_data = ascii.CreateFileStream (c_s);

  if (prot.compare ("TcpNewReno") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
    }
  else if (prot.compare ("TcpHybla") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHybla::GetTypeId ()));
    }
  else if (prot.compare ("TcpHighSpeed") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                          TypeIdValue (TcpHighSpeed::GetTypeId ()));
    }
  else if (prot.compare ("TcpVegas") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVegas::GetTypeId ()));
    }
  else if (prot.compare ("TcpScalable") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                          TypeIdValue (TcpScalable::GetTypeId ()));
    }
  else if (prot.compare ("TcpHtcp") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHtcp::GetTypeId ()));
    }
  else if (prot.compare ("TcpVeno") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVeno::GetTypeId ()));
    }
  else if (prot.compare ("TcpBic") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpBic::GetTypeId ()));
    }
  else if (prot.compare ("TcpYeah") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpYeah::GetTypeId ()));
    }
  else if (prot.compare ("TcpIllinois") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                          TypeIdValue (TcpIllinois::GetTypeId ()));
    }
  else if (prot.compare ("TcpWestwood") == 0)
    {
      // the default protocol type in ns3::TcpWestwood is WESTWOOD
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                          TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else if (prot.compare ("TcpWestwoodPlus") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                          TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
      Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else
    {
      NS_LOG_DEBUG ("Invalid TCP version");
      exit (1);
    }

  //
  // Explicitly create the nodes required by the topology (shown above).
  //
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c; // ALL Nodes
  c.Create (12);

  NodeContainer n0n5 = NodeContainer (c.Get (0), c.Get (5));
  NodeContainer n1n5 = NodeContainer (c.Get (1), c.Get (5));
  NodeContainer n2n5 = NodeContainer (c.Get (2), c.Get (5));
  NodeContainer n3n5 = NodeContainer (c.Get (3), c.Get (5));
  NodeContainer n4n5 = NodeContainer (c.Get (4), c.Get (5));

  NodeContainer n5n6 = NodeContainer (c.Get (5), c.Get (6));

  NodeContainer n7n6 = NodeContainer (c.Get (7), c.Get (6));
  NodeContainer n8n6 = NodeContainer (c.Get (8), c.Get (6));
  NodeContainer n9n6 = NodeContainer (c.Get (9), c.Get (6));
  NodeContainer n10n6 = NodeContainer (c.Get (10), c.Get (6));
  NodeContainer n11n6 = NodeContainer (c.Get (11), c.Get (6));

  //
  // Install Internet Stack
  //
  InternetStackHelper internet;
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue (rate));
  p2p.SetChannelAttribute ("Delay", StringValue (lat));
  NetDeviceContainer d0d5 = p2p.Install (n0n5);
  NetDeviceContainer d1d5 = p2p.Install (n1n5);
  NetDeviceContainer d2d5 = p2p.Install (n2n5);
  NetDeviceContainer d3d5 = p2p.Install (n3n5);
  NetDeviceContainer d4d5 = p2p.Install (n4n5);

  NetDeviceContainer d5d6 = p2p.Install (n5n6);

  NetDeviceContainer d7d6 = p2p.Install (n7n6);
  NetDeviceContainer d8d6 = p2p.Install (n8n6);
  NetDeviceContainer d9d6 = p2p.Install (n9n6);
  NetDeviceContainer d10d6 = p2p.Install (n10n6);
  NetDeviceContainer d11d6 = p2p.Install (n11n6);

  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i5 = ipv4.Assign (d0d5);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i5 = ipv4.Assign (d1d5);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i5 = ipv4.Assign (d2d5);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i5 = ipv4.Assign (d3d5);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i4i5 = ipv4.Assign (d4d5);

  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign (d5d6);

  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer i7i6 = ipv4.Assign (d7d6);

  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer i8i6 = ipv4.Assign (d8d6);

  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i6 = ipv4.Assign (d9d6);

  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer i10i6 = ipv4.Assign (d10d6);

  ipv4.SetBase ("10.1.11.0", "255.255.255.0");
  Ipv4InterfaceContainer i11i6 = ipv4.Assign (d11d6);

  NS_LOG_INFO ("Enable static global routing.");
  //
  // Turn on global static routing so we can actually be routed across the network.
  //
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Create Applications.");

  // TCP connfection from N0 to N7

  uint16_t sinkPorti7i6 = 8080;
  Address sinkAddressi7i6 (
      InetSocketAddress (i7i6.GetAddress (0), sinkPorti7i6)); // interface of n7
  PacketSinkHelper packetSinkHelperi7i6 ("ns3::TcpSocketFactory",
                                         InetSocketAddress (Ipv4Address::GetAny (), sinkPorti7i6));
  ApplicationContainer sinkAppsi7i6 = packetSinkHelperi7i6.Install (c.Get (7)); //n7 as sink
  sinkAppsi7i6.Start (Seconds (0.));
  sinkAppsi7i6.Stop (Seconds (100.));

  Ptr<Socket> ns3TcpSocketi7i6 =
      Socket::CreateSocket (c.Get (0), TcpSocketFactory::GetTypeId ()); //source at n0

  // Create TCP application at n0
  Ptr<MyApp> appi7i6 = CreateObject<MyApp> ();
  appi7i6->Setup (ns3TcpSocketi7i6, sinkAddressi7i6, 1040, 100000, DataRate ("250Kbps"));
  c.Get (0)->AddApplication (appi7i6);
  appi7i6->SetStartTime (Seconds (1.));
  appi7i6->SetStopTime (Seconds (100.));
  // --------------------------------------------
  // TCP connfection from N1 to N8
  uint16_t sinkPorti8i6 = 8081;
  Address sinkAddressi8i6 (
      InetSocketAddress (i8i6.GetAddress (0), sinkPorti8i6)); // interface of n8
  PacketSinkHelper packetSinkHelperi8i6 ("ns3::TcpSocketFactory",
                                         InetSocketAddress (Ipv4Address::GetAny (), sinkPorti8i6));
  ApplicationContainer sinkAppsi8i6 = packetSinkHelperi8i6.Install (c.Get (8)); //n8 as sink
  sinkAppsi8i6.Start (Seconds (0.));
  sinkAppsi8i6.Stop (Seconds (100.));

  Ptr<Socket> ns3TcpSocketi8i6 =
      Socket::CreateSocket (c.Get (1), TcpSocketFactory::GetTypeId ()); //source at n1

  // Create TCP application at n1
  Ptr<MyApp> appi8i6 = CreateObject<MyApp> ();
  appi8i6->Setup (ns3TcpSocketi8i6, sinkAddressi8i6, 1040, 100000, DataRate ("250Kbps"));
  c.Get (1)->AddApplication (appi8i6);
  appi8i6->SetStartTime (Seconds (1.));
  appi8i6->SetStopTime (Seconds (100.));
  // ---------------------------------------------

  // TCP connfection from N2 to N9
  uint16_t sinkPorti9i6 = 8082;
  Address sinkAddressi9i6 (
      InetSocketAddress (i9i6.GetAddress (0), sinkPorti9i6)); // interface of n9
  PacketSinkHelper packetSinkHelperi9i6 ("ns3::TcpSocketFactory",
                                         InetSocketAddress (Ipv4Address::GetAny (), sinkPorti9i6));
  ApplicationContainer sinkAppsi9i6 = packetSinkHelperi9i6.Install (c.Get (9)); //n9 as sink
  sinkAppsi9i6.Start (Seconds (0.));
  sinkAppsi9i6.Stop (Seconds (100.));

  Ptr<Socket> ns3TcpSocketi9i6 =
      Socket::CreateSocket (c.Get (2), TcpSocketFactory::GetTypeId ()); //source at n2

  // Create TCP application at n2
  Ptr<MyApp> appi9i6 = CreateObject<MyApp> ();
  appi9i6->Setup (ns3TcpSocketi9i6, sinkAddressi9i6, 1040, 100000, DataRate ("250Kbps"));
  c.Get (2)->AddApplication (appi9i6);
  appi9i6->SetStartTime (Seconds (1.));
  appi9i6->SetStopTime (Seconds (100.));
  // ---------------------------------------------
  // ---------------------------------------------

  // UDP connfection from N3 to N10

  uint16_t sinkPort2i10i6 = 6;
  Address sinkAddress2i10i6 (
      InetSocketAddress (i10i6.GetAddress (0), sinkPort2i10i6)); // interface of n10
  PacketSinkHelper packetSinkHelper2i10i6 (
      "ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort2i10i6));
  ApplicationContainer sinkApps2i10i6 = packetSinkHelper2i10i6.Install (c.Get (10)); //n10 as sink
  sinkApps2i10i6.Start (Seconds (0.));
  sinkApps2i10i6.Stop (Seconds (100.));

  Ptr<Socket> ns3UdpSocketi10i6 =
      Socket::CreateSocket (c.Get (3), UdpSocketFactory::GetTypeId ()); //source at n3

  // Create UDP application at n3
  Ptr<MyApp> app2i10i6 = CreateObject<MyApp> ();
  app2i10i6->Setup (ns3UdpSocketi10i6, sinkAddress2i10i6, 1040, 100000, DataRate ("250Kbps"));
  c.Get (3)->AddApplication (app2i10i6);
  app2i10i6->SetStartTime (Seconds (1.));
  app2i10i6->SetStopTime (Seconds (100.));
  // ---------------------------------------------

  // UDP connfection from N4 to N11

  uint16_t sinkPort2i11i6 = 7;
  Address sinkAddress2i11i6 (
      InetSocketAddress (i11i6.GetAddress (0), sinkPort2i11i6)); // interface of n11
  PacketSinkHelper packetSinkHelper2i11i6 (
      "ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort2i11i6));
  ApplicationContainer sinkApps2i11i6 = packetSinkHelper2i11i6.Install (c.Get (11)); //n11 as sink
  sinkApps2i11i6.Start (Seconds (0.));
  sinkApps2i11i6.Stop (Seconds (100.));

  Ptr<Socket> ns3UdpSocketi11i6 =
      Socket::CreateSocket (c.Get (4), UdpSocketFactory::GetTypeId ()); //source at n4

  // Create UDP application at n4
  Ptr<MyApp> app2i11i6 = CreateObject<MyApp> ();
  app2i11i6->Setup (ns3UdpSocketi11i6, sinkAddress2i11i6, 1040, 100000, DataRate ("250Kbps"));
  c.Get (4)->AddApplication (app2i11i6);
  app2i11i6->SetStartTime (Seconds (1.));
  app2i11i6->SetStopTime (Seconds (100.));

  // Increase UDP Rate
  // Simulator::Schedule (Seconds (30.0), &IncRate, app2, DataRate ("500kbps"));

  // Flow Monitor
  Ptr<FlowMonitor> flowmon;
  if (enableFlowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Schedule (Seconds (0.00001), &TraceCwnd, dumbell_cw_data);
  Simulator::Stop (Seconds (100.0));

  AnimationInterface anim ("dumbell_test.xml");
  anim.SetConstantPosition (c.Get (0), 5, 5);
  anim.SetConstantPosition (c.Get (1), 5, 10);
  anim.SetConstantPosition (c.Get (2), 5, 15);
  anim.SetConstantPosition (c.Get (3), 5, 20);
  anim.SetConstantPosition (c.Get (4), 5, 25);
  anim.SetConstantPosition (c.Get (5), 10, 15);
  anim.SetConstantPosition (c.Get (6), 15, 15);
  anim.SetConstantPosition (c.Get (7), 20, 5);
  anim.SetConstantPosition (c.Get (8), 20, 10);
  anim.SetConstantPosition (c.Get (9), 20, 15);
  anim.SetConstantPosition (c.Get (10), 20, 20);
  anim.SetConstantPosition (c.Get (11), 20, 25);

  Simulator::Run ();
  if (enableFlowMonitor)
    {
      flowmon->CheckForLostPackets ();
      flowmon->SerializeToXmlFile ("dumbell_test.flowmon", true, true);
    }

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
