/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

 // First communication (ARP discovery):
// 1. Node 1 broadcasts: "Who has node 4's IP address?" to the LAN.
// 2. Node 4 replies: "That's me! Here's my MAC address."
// 3. node 1 sends the packet to node 4's MAC address.
// Then the reply direction:
// 4. Node 4 broadcasts: "Who has node 1's IP address?" to the LAN.
// 5. Node 1 replies with its MAC address.
// 6. Node 4 sends the packet to node 1's MAC address.
// note that this only happens the first time a node send sa packet, 
// it will send the packet direclty to the other node's MAC address after that, until the ARP cache expires (10 seconds by default)



#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/netanim-module.h"


// Default Network Topology
//
//       172.16.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 172.16.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma = 3; // number of csma nodes to create

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma; // if nCsma is 0, set it to 1 to avoid creating a network with no CSMA nodes

    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1)); // node1 is either get(1) for p2p or get(0) for csma
    csmaNodes.Create(nCsma); 

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    CsmaHelper csma; 
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    InternetStackHelper stack;

    // Disabling IPv6 because it is not necessary to show what we want to demonstrate here.
    // Note: Normal networks typically have both IPv4 and IPv6 enabled.
    stack.SetIpv6StackInstall(false);

    stack.Install(p2pNodes.Get(0));
    stack.Install(csmaNodes);

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Ipv4AddressHelper address;
    address.SetBase("172.16.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("172.16.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    UdpEchoServerHelper echoServer(1234); // port number, server will listen on this port

    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma)); // last node in csmaNodes is the server, 
    serverApps.Start(Seconds(1));
    serverApps.Stop(Seconds(10));

    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), 1234);
    echoClient.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.001)));
    echoClient.SetAttribute("PacketSize", UintegerValue(2024)); //seems like it needs to send twice in a row cuz cap is 1500 

    ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0)); // first node in p2pNodes is the client
    clientApps.Start(Seconds(2));
    clientApps.Stop(Seconds(10));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables(); //this is global routing, which means it will automatically calculate the routing tables for all nodes in the network.

    pointToPoint.EnablePcapAll("p2p"); //pcap file shows packets send and received on p2p, can be opened in wireshark, named second-0-0.pcap and second-1-0.pcap, (cuz p2p has 2 nodes)
    csma.EnablePcap("csma1", csmaDevices.Get(1), true); // same but with csma, only one file named second-2-0.pcap
    csma.EnablePcap("csma3", csmaDevices.Get(3), true);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(p2pNodes.Get(0));
    mobility.Install(csmaNodes);
    AnimationInterface anim("second-test-netanim.xml");
    anim.SetConstantPosition(p2pNodes.Get(0), 10.0, 10.0);
    anim.SetConstantPosition(csmaNodes.Get(0), 17.0, 25.0);
    anim.SetConstantPosition(csmaNodes.Get(1), 15.0, 30.0);
    anim.SetConstantPosition(csmaNodes.Get(2), 40.0, 10.0);
    anim.SetConstantPosition(csmaNodes.Get(3), 43.0, 30.0);
    



        AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("p2p-second-test.tr"));
    csma.EnableAsciiAll(ascii.CreateFileStream("csma-second-test.tr"));
    Simulator::Stop(Seconds(20));



    Simulator::Run();

    monitor->CheckForLostPackets();
    monitor->SerializeToXmlFile("second-flowmonitor.xml", true, true);



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
