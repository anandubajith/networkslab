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
    NodeContainer nodes;
    nodes.create(13);
    /* {0,1}     = CNC servers
     * {2,3,4,5} = CCC Desktop lan
     * {6,7,8}   = CCC Laptops
     * {9,10,11} = Routers
     * {12} = WiFi AP
     */

    /*
     * Setup physical connections
     */


    /*
     * WiFi in CCC Room
     */
    NetDeviceContainer cccWifiDevices;
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
    // todo: pass node 12
    cccWifiDevices = wifi.Install(phy, mac, ccc_laptop_nodes);
    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, wifiApNode);

    MobilityHelper mobility;

    mobility.SetPositionAllocator(
            "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0),
                                          "MinY", DoubleValue(0.0),
                                          "DeltaX", DoubleValue(5.0),
                                          "DeltaY", DoubleValue(10.0),
                                          "GridWidth", UintegerValue(3),
                                          "LayoutType", StringValue("RowFirst")
    );

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
            RectangleValue(Rectangle(-50, 50, -50, 50)));
    // todo: pass nodes 6,7,8
    mobility.Install(ccc_laptop_nodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);

    /*
     * Setup Networking stack and routing
     */
    InternetStackHelper stack;
    stack.Install (csmaNodes);
    stack.Install (wifiApNode);
    stack.Install (wifiStaNodes);

    Ipv4AddressHelper address;

    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pDevices);

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign (csmaDevices);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    address.Assign (staDevices);
    address.Assign (apDevices);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /*
     * Setup applications
     */
    // Server1 -> FTP application to one of CCC help desk
    uint16_t port = 21;
    // FTP Server on Server 1
    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(i.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(10000)); // send 10000 bytes
    ApplicationContainer sourceApps = source.Install(cnc_lan.Get(0));
    sourceApps.Start(Seconds(0.0));
    sourceApps.Stop(Seconds(10.0));
    // FTP Client on CCC Desktop LAN
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(ccc_desktop_lan.Get(2));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(30.0));

    // Server2 -> CBR application to one of laptop devices
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(ccc_laptop_nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(i.GetAddress(2), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(cnc_lan.Get(1));


    /*
     * Setup tracing
     */
    phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    pointToPoint.EnablePcapAll ("third");
    phy.EnablePcap ("third", apDevices.Get (0));
    csma.EnablePcap ("third", csmaDevices.Get (0), true);

    /*
     * Run simulation
     */
    Simulator::Stop(Seconds(100));
    Simulator::Run();
    Simulator::Destroy();
    return 0;

}
