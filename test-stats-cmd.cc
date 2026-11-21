/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

// for the cmd stats we need flowmonitor, so include it and add the 2lines for helper, 
// and we copy paste that long code, thats about it

//for netanim, we need include the (2)module, and add the 2 lines for mobility and animationinterface, 
// and set the position of the nodes, set the xml file name too also


#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/flow-monitor-module.h"

#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"


// this one is first.cc, 2clients 2 servers, cmd stats, complex topo.










// Default Network Topology
// djiksta (shortest path first algorithm)
//       10.1.1.0    10.1.2.0    10.1.3.0    10.1.4.0
//    n0 -------- n1 -------- n2 -------- n3 ---------n4
//    point-to-point  p2p         p2p         p2p
//
//we could try another topology? maybe try to have more than two connection for one node,
//         
//     n0 ---- n1 --- n3
//       -     -    -   -
//          -  -  -       -
//            n2           n4
//             
// would it work the same? would it need a lot of changes? //just add devicecontainer and assign the ip address.

// so basically u need a routing option, either have static or global, and to keep it pointtopoint it needs to be a pair of devices i believe?


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(5);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    
    NetDeviceContainer device01 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer device02 = pointToPoint.Install(nodes.Get(0), nodes.Get(2));
    NetDeviceContainer device12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer device13 = pointToPoint.Install(nodes.Get(1), nodes.Get(3));
    NetDeviceContainer device23 = pointToPoint.Install(nodes.Get(2), nodes.Get(3));
    NetDeviceContainer device34 = pointToPoint.Install(nodes.Get(3), nodes.Get(4));

    //devices.Add(pointToPoint.Install(nodes.Get(0), nodes.Get(1)));
    //devices.Add(pointToPoint.Install(nodes.Get(1), nodes.Get(2)));
    //devices.Add(pointToPoint.Install(nodes.Get(2), nodes.Get(3)));
    //devices.Add(pointToPoint.Install(nodes.Get(3), nodes.Get(4)));


    InternetStackHelper stack;
    Ipv4GlobalRoutingHelper globalRouting;
    stack.SetRoutingHelper(globalRouting);

    // Disableing IPv6 because it is not necessary to show what we want to demonstrate here.
    // Note:Normal networks typically have both IPv4 and IPv6 enabled.
    stack.SetIpv6StackInstall(false);

    stack.Install(nodes);

    //also need this for the cmd stats to work, it needs flowmonitor in general
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer int01 = address.Assign(device01);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer int12 = address.Assign(device12);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer int23 = address.Assign(device23);
    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer int34 = address.Assign(device34);
    address.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer int02 = address.Assign(device02);
    address.SetBase("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer int13 = address.Assign(device13);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // AsciiTraceHelper ascii; //to comment ctrl+shift+7
    // Ptr<OutputStreamWrapper> routingStream =
    // ascii.CreateFileStream("routing-tables.txt");

    // Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(
    // Seconds(0.1), routingStream);

//3689 is a port number. It is the port number that the server will listen on and the client will send to.
    UdpEchoServerHelper echoServer(3689);
    UdpEchoServerHelper echoServer2(9);
//first node is nodes.get(0) and second node is nodes.get(1). The server is installed on the second node.
//server runs at 1s then stops at 10s, while client sends at 2s, 
// so we need to have the other nodes also send to server maybe???
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0)); 
    serverApps.Start(Seconds(1));
    serverApps.Stop(Seconds(20));
    ApplicationContainer serverApps2 = echoServer2.Install(nodes.Get(3)); 
    serverApps2.Start(Seconds(1));
    serverApps2.Stop(Seconds(20));

    UdpEchoClientHelper echoClient(int01.GetAddress(0), 3689); //even if this is auto it will still recieve the ip closest, so not 4.1 but 3.2, just like the static ig
    echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.0001)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    UdpEchoClientHelper echoClient2(int23.GetAddress(1), 9); 
    echoClient2.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(0.0001)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(4));
    clientApps.Start(Seconds(2));
    clientApps.Stop(Seconds(20));
    ApplicationContainer clientApps2 = echoClient2.Install(nodes.Get(2));
    clientApps2.Start(Seconds(2));
    clientApps2.Stop(Seconds(20));

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
    AnimationInterface anim("test-pkts.xml");
    anim.SetConstantPosition(nodes.Get(0), 10.0, 10.0);
    anim.SetConstantPosition(nodes.Get(1), 17.0, 25.0);
    anim.SetConstantPosition(nodes.Get(2), 15.0, 30.0);
    anim.SetConstantPosition(nodes.Get(3), 40.0, 10.0);
    anim.SetConstantPosition(nodes.Get(4), 43.0, 30.0);


    //AsciiTraceHelper is used to create an ASCII trace file that records the events in the simulation. The trace file can be used for debugging and analysis of the simulation results.
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("test-pkts.tr"));
    Simulator::Stop(Seconds(20));

    Simulator::Run();

    // Check and export FlowMonitor statistics
    monitor->CheckForLostPackets();
    monitor->SerializeToXmlFile("test-pkts-flowmonitor.xml", true, true);




    
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
