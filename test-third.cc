/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
// Default Network Topology
//
//   Wifi 172.11.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    172.11.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 172.11.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma = 3;
    uint32_t nWifi = 3;
    bool tracing = true;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("nWifi", "Number of wifi STA devices", nWifi);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    cmd.Parse(argc, argv);

    // The underlying restriction of 18 is due to the grid position
    // allocator's configuration; the grid layout will exceed the
    // bounding box if more than 18 nodes are provided.
    if (nWifi > 18)
    {
        std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box"
                  << std::endl;
        return 1;
    }

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1));
    csmaNodes.Create(nCsma);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    NodeContainer wifiStaNodes; // stations are like phones and laptops, they connect to the access point, which is like a router
    wifiStaNodes.Create(nWifi);
    NodeContainer wifiApNode = p2pNodes.Get(0); // the access point is like a router

    // yans= yet another network simulator
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default(); // 
    YansWifiPhyHelper phy; 
    phy.SetChannel(channel.Create());
    //the channel is used to connect the stations and the access point
    //the phy layer is used to send and receive packets,
    
    WifiMacHelper mac; // 
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;

    NetDeviceContainer staDevices; //stations
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes); // kinda like the address thing the way we set it after altering it
    // this installs the devices on the station, with the correct type, also
    // active probing is set to false, cuz its an ap thing not a station thing

    // the ap does active probing, the station just listens for the ap to say 
    //hey im here, and then it connects to it
    NetDeviceContainer apDevices; //access point devices, there is only one ap device, but there are multiple sta devices
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, wifiApNode);

    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);

    AnimationInterface anim("third-test-netanim.xml");
    // anim.SetConstantPosition(p2pNodes.Get(0), 50.0, 50.0);
    // anim.SetConstantPosition(p2pNodes.Get(1), 60.0, 60.0);
    // anim.SetConstantPosition(csmaNodes.Get(1), 70.0, 60.0);
    // anim.SetConstantPosition(csmaNodes.Get(2), 80.0, 50.0);
    // anim.SetConstantPosition(csmaNodes.Get(3), 90.0, 70.0);
    // anim.SetConstantPosition(wifiStaNodes.Get(0), 40.0, 40.0);
    // anim.SetConstantPosition(wifiStaNodes.Get(1), 40.0, 30.0);
    // anim.SetConstantPosition(wifiStaNodes.Get(2), 30.0, 50.0);


    InternetStackHelper stack;

    // Disabling IPv6 because it is not necessary to show what we want to demonstrate here.
    // Note: Normal networks typically have both IPv4 and IPv6 enabled.
    stack.SetIpv6StackInstall(false);

    stack.Install(csmaNodes);
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Ipv4AddressHelper address;

    address.SetBase("172.11.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("172.11.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    address.SetBase("172.11.3.0", "255.255.255.0");
    address.Assign(staDevices);
    address.Assign(apDevices);

    UdpEchoServerHelper echoServer(3456);

    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1));
    serverApps.Stop(Seconds(10));

    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), 3456);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));
    echoClient.SetAttribute("PacketSize", UintegerValue(2048));

    ApplicationContainer clientApps = echoClient.Install(wifiStaNodes.Get(nWifi - 1));
    clientApps.Start(Seconds(2));
    clientApps.Stop(Seconds(10));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("third-p2p.tr"));
    csma.EnableAsciiAll(ascii.CreateFileStream("third-csma.tr"));
    phy.EnableAsciiAll(ascii.CreateFileStream("third-wifi.tr"));


    Simulator::Stop(Seconds(10));

    if (tracing)
    {
        phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        pointToPoint.EnablePcapAll("third-p2p"); //btw we use the helpers to enable pcap and tracing
        phy.EnablePcap("third-wifi", apDevices.Get(0)); //
        csma.EnablePcap("third-csma", csmaDevices.Get(0), true); 
    }




    Simulator::Run();


// start of the cmd stats here
//its a loop for every flow id, plus the total at the end
    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    int j = 0;
    float AvgThroughput = 0;
    Time Delay;

    NS_LOG_UNCOND("\n----------Results output ID wise----------");

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator
        iter = stats.begin();
        iter != stats.end();
        ++iter)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);
        NS_LOG_UNCOND("\n----------Flow ID: " << iter->first);
        NS_LOG_UNCOND("Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
        NS_LOG_UNCOND("Sent Packets = "<< iter->second.txPackets);
        NS_LOG_UNCOND("Received Packets = "<< iter->second.rxPackets);
        NS_LOG_UNCOND("Throughput = " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024 << " Kbps");
        NS_LOG_UNCOND("Delay = "<< iter->second.delaySum);


        SentPackets = SentPackets + (iter->second.txPackets);
        ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
        AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024);
        Delay = Delay + (iter->second.delaySum);

        j = j + 1; //flow id increment
    }
    AvgThroughput = AvgThroughput/j;
    NS_LOG_UNCOND("\n----------Total Results of the simulation----------");

    NS_LOG_UNCOND("Total Sent Packets = " << SentPackets);
    NS_LOG_UNCOND("Total Received Packets = " << ReceivedPackets);
    
    NS_LOG_UNCOND("Average Throughput = " << AvgThroughput);
    NS_LOG_UNCOND("Delay = " << Delay);

    NS_LOG_UNCOND("Total Flow Id = " << j);
//end of the cmd stats here


    Simulator::Destroy();
    return 0;
}
