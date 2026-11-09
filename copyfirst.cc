/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"

// Default Network Topology
//
//       10.1.1.0    10.1.2.0    10.1.3.0    10.1.4.0
//    n0 -------- n1 -------- n2 -------- n3 ---------n4
//    point-to-point  p2p         p2p         p2p
//
#include "ns3/netanim-module.h"

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
    NetDeviceContainer device12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer device23 = pointToPoint.Install(nodes.Get(2), nodes.Get(3));
    NetDeviceContainer device34 = pointToPoint.Install(nodes.Get(3), nodes.Get(4));

    //devices.Add(pointToPoint.Install(nodes.Get(0), nodes.Get(1)));
    //devices.Add(pointToPoint.Install(nodes.Get(1), nodes.Get(2)));
    //devices.Add(pointToPoint.Install(nodes.Get(2), nodes.Get(3)));
    //devices.Add(pointToPoint.Install(nodes.Get(3), nodes.Get(4)));



    Ipv4StaticRoutingHelper staticRouting; //routing also
    InternetStackHelper stack;
    stack.SetRoutingHelper(staticRouting); //routing
    // Disableing IPv6 because it is not necessary to show what we want to demonstrate here.
    // Note:Normal networks typically have both IPv4 and IPv6 enabled.
    stack.SetIpv6StackInstall(false);

    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer int01 = address.Assign(device01);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer int12 = address.Assign(device12);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer int23 = address.Assign(device23);
    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer int34 = address.Assign(device34);


    //static routing ting
    //can be a loop for each i (0 to 4)??idk i forgor
    // maybe : Ptr<Ipv4StaticRouting> routingi = staticRouting.GetStaticRouting(Ptr<Ipv4> ipv4_i = nodes.Get(i)->GetObject<Ipv4>());
    Ptr<Ipv4> ipv4_0 = nodes.Get(0)->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4_1 = nodes.Get(1)->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4_2 = nodes.Get(2)->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4_3 = nodes.Get(3)->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4_4 = nodes.Get(4)->GetObject<Ipv4>();


    Ptr<Ipv4StaticRouting> routing0 = staticRouting.GetStaticRouting(ipv4_0);
    Ptr<Ipv4StaticRouting> routing1 = staticRouting.GetStaticRouting(ipv4_1);
    Ptr<Ipv4StaticRouting> routing2 = staticRouting.GetStaticRouting(ipv4_2);
    Ptr<Ipv4StaticRouting> routing3 = staticRouting.GetStaticRouting(ipv4_3);
    Ptr<Ipv4StaticRouting> routing4 = staticRouting.GetStaticRouting(ipv4_4);





// Node 0: Route towards Node 4
routing0->AddNetworkRouteTo(
    Ipv4Address("10.1.4.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.1.2"), //node1
    ipv4_0->GetInterfaceForDevice(device01.Get(0))
);

//0->2
routing0->AddNetworkRouteTo(
    Ipv4Address("10.1.2.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.1.2"), //node1
    ipv4_0->GetInterfaceForDevice(device01.Get(0))
);

//0->3
routing0->AddNetworkRouteTo(
    Ipv4Address("10.1.3.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.1.2"), //node1
    ipv4_0->GetInterfaceForDevice(device01.Get(0))
);

// Node 1: Route towards Node 4
routing1->AddNetworkRouteTo(
    Ipv4Address("10.1.4.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.2.2"), //node2
    ipv4_1->GetInterfaceForDevice(device12.Get(0))
);

//1->3
routing1->AddNetworkRouteTo(
    Ipv4Address("10.1.3.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.2.2"), //node2
    ipv4_1->GetInterfaceForDevice(device12.Get(0))
);


// Node 2: Routes towards Node 4 AND Node 0
routing2->AddNetworkRouteTo(
    Ipv4Address("10.1.4.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.3.2"), //node3
    ipv4_2->GetInterfaceForDevice(device23.Get(0))
);

routing2->AddNetworkRouteTo(
    Ipv4Address("10.1.1.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.2.1"), //node1
    ipv4_2->GetInterfaceForDevice(device12.Get(1))
);


// Node 3: Route towards Node 0
routing3->AddNetworkRouteTo(
    Ipv4Address("10.1.1.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.3.1"), //node2
    ipv4_3->GetInterfaceForDevice(device23.Get(1))
);

//3->1
routing3->AddNetworkRouteTo(
    Ipv4Address("10.1.2.0"),
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.3.1"), //node2
    ipv4_3->GetInterfaceForDevice(device23.Get(1))
);


// Node 4: Route towards Node 0
routing4->AddNetworkRouteTo(
    Ipv4Address("10.1.1.0"), //network01
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.4.1"), //node3
    ipv4_4->GetInterfaceForDevice(device34.Get(1))
);

//4->2
routing4->AddNetworkRouteTo(
    Ipv4Address("10.1.3.0"), 
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.4.1"), //node3
    ipv4_4->GetInterfaceForDevice(device34.Get(1))
);

//4->1
routing4->AddNetworkRouteTo(
    Ipv4Address("10.1.2.0"), 
    Ipv4Mask("255.255.255.0"),
    Ipv4Address("10.1.4.1"), //node3
    ipv4_4->GetInterfaceForDevice(device34.Get(1))
);


    //this the end of the static routing ting


//3689 is a port number. It is the port number that the server will listen on and the client will send to.
    UdpEchoServerHelper echoServer(3689);
//first node is nodes.get(0) and second node is nodes.get(1). The server is installed on the second node.
//server runs at 1s then stops at 10s, while client sends at 2s, 
// so we need to have the other nodes also send to server maybe???
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(4)); //to change server node change here
    serverApps.Start(Seconds(1));
    serverApps.Stop(Seconds(20));
    // when changing getaddress()to 0, it will still recieve the set last line in routing so ip of first send might not be the same as the recieve of client
    UdpEchoClientHelper echoClient(int34.GetAddress(1), 3689); //change the interface and the position either 0 or 1 as per position in interface
    echoClient.SetAttribute("MaxPackets", UintegerValue(1)); 
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0)); //even if client node is server node, its diff port so it still works!!!
    clientApps.Start(Seconds(2));
    clientApps.Stop(Seconds(20));



    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
    AnimationInterface anim("copyfirst-static.xml");
    anim.SetConstantPosition(nodes.Get(0), 10.0, 10.0);
    anim.SetConstantPosition(nodes.Get(1), 20.0, 20.0);
    anim.SetConstantPosition(nodes.Get(2), 30.0, 30.0);
    anim.SetConstantPosition(nodes.Get(3), 40.0, 40.0);
    anim.SetConstantPosition(nodes.Get(4), 50.0, 50.0);

//AsciiTraceHelper is used to create an ASCII trace file that records the events in the simulation. The trace file can be used for debugging and analysis of the simulation results.
    //AsciiTraceHelper ascii;
    //pointToPoint.EnableAsciiAll(ascii.CreateFileStream("copyfirst-5ver.tr"));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
