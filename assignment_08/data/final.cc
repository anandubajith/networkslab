#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/rectangle.h"
#include "ns3/yans-wifi-helper.h"
#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
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
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

using namespace ns3;

int main() {
    NetDeviceContainer allDevices;

    NodeContainer nodes;
    nodes.Create(13);

    NodeContainer cncServerNodes;
    cncServerNodes.Add(nodes.Get(0));
    cncServerNodes.Add(nodes.Get(1));

    NodeContainer cccDesktopNodes;
    cccDesktopNodes.Add(nodes.Get(2));
    cccDesktopNodes.Add(nodes.Get(3));
    cccDesktopNodes.Add(nodes.Get(4));
    cccDesktopNodes.Add(nodes.Get(5));

    NodeContainer cccLaptopNodes;
    cccLaptopNodes.Add(nodes.Get(6));
    cccLaptopNodes.Add(nodes.Get(7));
    cccLaptopNodes.Add(nodes.Get(8));

    NodeContainer routerNodes;
    routerNodes.Add(nodes.Get(9));
    routerNodes.Add(nodes.Get(10));
    routerNodes.Add(nodes.Get(11));

    NodeContainer wifiApNodes;
    wifiApNodes.Add(nodes.Get(12));

    /*
     * Setup physical connections
     */
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));
    NetDeviceContainer cccDesktopDevices;
    cccDesktopDevices = csma.Install(cccDesktopNodes);
    allDevices.Add(cccDesktopDevices);

    // todo: add router
    // todo: add server

    /*
     * WiFi in CCC Room
     */
    NetDeviceContainer cccLaptopDevices;
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("dss-room");
    WifiHelper wifi;
    mac.SetType("ns3::StaWifiMac",
                            "Ssid", SsidValue(ssid),
                            "ActiveProbing", BooleanValue(false)
    );
    cccLaptopDevices = wifi.Install(phy, mac, cccLaptopNodes);
    allDevices.Add(cccLaptopDevices);
    NetDeviceContainer wifiApDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    wifiApDevices = wifi.Install(phy, mac, wifiApNodes.Get(0));
    allDevices.Add(wifiApDevices);

    MobilityHelper mobility;

    mobility.SetPositionAllocator(
            "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0),
                                          "MinY", DoubleValue(0.0),
                                          "DeltaX", DoubleValue(5.0),
                                          "DeltaY", DoubleValue(10.0),
                                          "GridWidth", UintegerValue(3),
                                          "LayoutType", StringValue("RowFirst")
    );

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                        "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50))
    );
    mobility.Install(cccLaptopNodes);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodes);

    /*
     * Setup Networking stack and routing
     */
    InternetStackHelper stack;
    stack.Install (nodes);


    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer deviceInterfaces;
    deviceInterfaces = address.Assign (allDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /*
     * Setup applications
     */
    // Server1 -> FTP application to one of CCC help desk
    uint16_t port = 21;
    // FTP Server on Server 1
    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(deviceInterfaces.GetAddress(0), port));
    source.SetAttribute("MaxBytes", UintegerValue(10000)); // send 10000 bytes
    ApplicationContainer sourceApps = source.Install(cncServerNodes.Get(0));
    sourceApps.Start(Seconds(0.0));
    sourceApps.Stop(Seconds(10.0));
    // FTP Client on CCC Desktop LAN
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(cccDesktopNodes.Get(2));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(30.0));

    // Server2 -> CBR application to one of laptop devices
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(cccLaptopNodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(deviceInterfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(cncServerNodes.Get(1));


    /*
     * Setup tracing
     */
    phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    /* pointToPoint.EnablePcapAll ("third"); */
    phy.EnablePcap ("third", wifiApDevices.Get (0));
    /* csma.EnablePcap ("third", csmaDevices.Get (0), true); */

    /*
     * Run simulation
     */
    Simulator::Stop(Seconds(100));
    Simulator::Run();
    Simulator::Destroy();
    return 0;

}
