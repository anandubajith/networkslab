#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/rectangle.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

int main() {
  NodeContainer cnc_lan; // 2 servers
  cnc_lan.Create(2);

  NodeContainer ccc_desktop_lan; // 4 desktop
  ccc_desktop_lan.Create(4);

  NodeContainer ccc_laptop_nodes; // 4 desktop
  ccc_laptop_nodes.Create(3);

  NodeContainer router_nodes;
  router_nodes.Create(3);

  cnc_lan.Add(router_nodes.Get(0));
  ccc_desktop_lan.Add(router_nodes.Get(2));

  // Connection in CNC LAN
  CsmaHelper csma1;
  csma1.SetChannelAttribute("DataRate", StringValue("5Mbps"));
  // todo: do we need delay set?
  NetDeviceContainer cncLanDevices;
  cncLanDevices = csma1.Install(cnc_lan);

  // Connection in CCC Desktops
  CsmaHelper csma2;
  csma2.SetChannelAttribute("DataRate", StringValue("5Mbps"));
  NetDeviceContainer ccc_desktop_lan_devices;
  ccc_desktop_lan_devices = csma1.Install(ccc_desktop_lan);

  // Connection between R0 and R1 with 2mbps and 20ms delay
  PointToPointHelper R0toR1pointToPoint;
  R0toR1pointToPoint.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
  R0toR1pointToPoint.SetChannelAttribute("Delay", StringValue("20ms"));

  // Connection between R1 and R2 with faulty cable loss probability 0.15
  PointToPointHelper R1toR2pointToPoint;
  R1toR2pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  R1toR2pointToPoint.SetChannelAttribute("Delay", StringValue("20ms"));

  // Connection between R1 and WiFI
  PointToPointHelper R1toWiFipointToPoint;
  R1toWiFipointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  R1toWiFipointToPoint.SetChannelAttribute("Delay", StringValue("20ms"));

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiMacHelper mac;
  Ssid ssid = Ssid("dss-room");

  WifiHelper wifi;

  NetDeviceContainer cccWifiDevices;
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
              BooleanValue(false));
  cccWifiDevices = wifi.Install(phy, mac, ccc_laptop_nodes);

  NetDeviceContainer apDevices;
  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  apDevices = wifi.Install(phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(5.0), "DeltaY", DoubleValue(10.0),
      "GridWidth", UintegerValue(3), "LayoutType", StringValue("RowFirst"));

  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
                            RectangleValue(Rectangle(-50, 50, -50, 50)));
  mobility.Install(ccc_laptop_nodes);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);

  // force routers to build routing table
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Applications
  // Server1 -> FTP application to one of CCC help desk
  uint16_t port = 21;
  // FTP Server on Server 1
  BulkSendHelper source("ns3::TcpSocketFactory",
                        InetSocketAddress(i.GetAddress(1), port));
  source.SetAttribute("MaxBytes", UintegerValue(10000)); // send 10000 bytes
  ApplicationContainer sourceApps = source.Install(cnc_lan.Get(0));
  sourceApps.Start(Seconds(0.0));
  sourceApps.Stop(Seconds(10.0));
  // FTP Client on CCC Desktop LAN
  PacketSinkHelper sink("ns3::TcpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApps = sink.Install(ccc_desktop_lan.Get(2));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(30.0));

  // Server2 -> CBR application to one of laptop devices
  UdpEchoServerHelper echoServer(9);
  ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient(i.GetAddress(2), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(cnc_lan.Get(1));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  // Run simulation for 100 seconds
  Simulator::Stop(Seconds(100));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
