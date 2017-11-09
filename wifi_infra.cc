/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/propagation-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include <iostream>
#include <sstream>
#include <math.h>

using namespace ns3;
using namespace std;

//running settings
uint32_t nNodes = 10; //GLOBAL
uint32_t transportMode = 0; //UDP
static bool printLog = true;
bool traffic = true;
uint32_t packetsize;
double runningTime = 60.0;
double offTime = 0.1;
double onTime = 0.1;

size_t nearestNode = -1;
size_t farthestNode = -1;
double nearestNodeDistance;
double farthestNodeDistance;
char prefix[150] = "";


double calcDistance (uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
  return sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void findNode(uint32_t ApX, uint32_t ApY, uint32_t NodeX, uint32_t NodeY, size_t id){
  double result = calcDistance(ApX, ApY, NodeX, NodeY);
  if(id == 0){
    nearestNode = id;
    farthestNode = id;
    nearestNodeDistance = result;
    farthestNodeDistance = result;
  }
  else{
    if(result > farthestNodeDistance){
      farthestNode = id;
      farthestNodeDistance = result;
    }
    if(result < nearestNodeDistance){
      nearestNode = id;
      nearestNodeDistance = result;
    }
  }
}

int ns3Rand(int min, int max){

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min));
  x->SetAttribute ("Max", DoubleValue (max));

  int value = x->GetValue ();

  return value;
}

void setMobility(NodeContainer &apnode, NodeContainer &nodes) {

  MobilityHelper mobilityh;
  //List of points
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  //Set apnode static
  uint32_t ApX = 50;
  uint32_t ApY = 50;
  positionAlloc->Add (Vector (ApX, ApY, 0.0));
  mobilityh.SetPositionAllocator (positionAlloc);
  mobilityh.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityh.Install (apnode);
  //Set station node static
  for (size_t i = 0; i < nNodes; i++) {
    uint32_t NodeX = ns3Rand(0, 100);
    uint32_t NodeY = ns3Rand(0, 100);
    findNode(ApX, ApY, NodeX, NodeY, i);
    positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (NodeX, NodeY, 0.0));
    mobilityh.SetPositionAllocator (positionAlloc);
    mobilityh.Install (nodes.Get(i));
  }
  //cout << "Mais Distante: " << farthestNode << "     com a distancia de " << dfarthestNode << "." << std::endl;
  //cout << "Mais Proximo : " << nearestNode << "    com a distancia de " << dnearestNode << "." << std::endl;
  //Set list of points

}

void installP2PDevices(NodeContainer &p2pnode, NetDeviceContainer &p2pdevice){
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  //Average delay according to literature -- offset point to point connection
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  p2pdevice = pointToPoint.Install (p2pnode);
}

void installWirelesp2pdevice(NodeContainer &apnode, NodeContainer &nodes, NetDeviceContainer &apdevice, NetDeviceContainer &devices){

  // 5. Install wireless devices
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");

  YansWifiChannelHelper wifiChannel;
  wifiChannel = YansWifiChannelHelper::Default ();

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

  Ssid ssid = Ssid ("wifi-default");

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);
  // setup ap.
  wifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
  apdevice = wifi.Install (wifiPhy, wifiMac, apnode);
}

void installInternetProtocol(NodeContainer &apnode, NodeContainer &nodes, NodeContainer &p2pnode, NetDeviceContainer &apdevice, NetDeviceContainer &devices, NetDeviceContainer &p2pdevice, Ipv4InterfaceContainer &apdeviceIP, Ipv4InterfaceContainer &devicesIP, Ipv4InterfaceContainer &p2pdeviceIP){

  // 6. Install TCP/IP stack & assign IP addresses
  InternetStackHelper internet;
  internet.Install (nodes);
  internet.Install (apnode);
  internet.Install (p2pnode.Get(0));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("198.162.10.0", "255.255.255.0");
  apdeviceIP = ipv4.Assign (apdevice);
  devicesIP = ipv4.Assign (devices);

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  p2pdeviceIP = ipv4.Assign (p2pdevice);
}

void installUDPCommunication(NodeContainer &nodes, Ipv4InterfaceContainer &p2pdeviceIP){
  ApplicationContainer cbrApps;

  //onTime and OffTime settings
  std::ostringstream ossOnTime;
  ossOnTime << "ns3::ConstantRandomVariable[Constant=" << onTime << "]";
  std::ostringstream ossOffTime;
  ossOffTime << "ns3::ConstantRandomVariable[Constant=" << offTime << "]";

  for (uint32_t i = 0; i < nNodes; i++){
    OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (p2pdeviceIP.GetAddress(0), i+1000));
    onOffHelper.SetAttribute ("PacketSize", UintegerValue (packetsize));
    onOffHelper.SetAttribute ("OnTime",StringValue(ossOnTime.str()));
    onOffHelper.SetAttribute ("OffTime",StringValue(ossOffTime.str()));

    onOffHelper.SetAttribute ("DataRate", StringValue ("512kbps"));
    cbrApps.Add (onOffHelper.Install (nodes.Get(i)));
  }

  // Start and stop time of application
  cbrApps.Start(Seconds(1.0));
  cbrApps.Stop(Seconds(runningTime+1));
}

void installTCPCommunication(NodeContainer &nodes, NodeContainer &p2pnode, Ipv4InterfaceContainer &p2pdeviceIP){
  ApplicationContainer serverApp;
  ApplicationContainer sinkApp;

  std::ostringstream ossOnTime;
  ossOnTime << "ns3::ConstantRandomVariable[Constant=" << onTime << "]";
  std::ostringstream ossOffTime;
  ossOffTime << "ns3::ConstantRandomVariable[Constant=" << offTime << "]";

  /* Install TCP/UDP Transmitter on the station */
  for (uint32_t i = 0; i < nNodes; i++){
  /* Install TCP Receiver on the server - p2p device */
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (p2pdeviceIP.GetAddress(0), i+10000));
    sinkApp = sinkHelper.Install (nodes.Get(i));
    sinkApp.Add (sinkHelper.Install (p2pnode.Get(0)));
    sinkApp.Start (Seconds (0.0));
    OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (p2pdeviceIP.GetAddress(0), i+10000)));
    server.SetAttribute ("PacketSize", UintegerValue (packetsize));
    server.SetAttribute ("OnTime", StringValue(ossOnTime.str()));
    server.SetAttribute ("OffTime", StringValue(ossOffTime.str()));
    server.SetAttribute ("DataRate", StringValue ("512kbps"));
    serverApp = server.Install (nodes.Get(i));
    serverApp.Start (Seconds (1));
  }
    serverApp.Stop(Seconds(runningTime+1));
}

void buildStatistics(FlowMonitorHelper &flowmon, Ptr<FlowMonitor> &monitor, Ipv4InterfaceContainer &devicesIP, Ipv4InterfaceContainer &p2pdeviceIP){

  double throughput = 0;
  double delay = 0;
  double sumThroughput = 0.0;
  double meanThroughput = 0.0;
  double meanDelayPackets = 0.0;
  double meanLostPackets = 0.0;
  int count = 0;
  FILE *f;

  monitor->CheckForLostPackets();
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());

  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i=stats.begin (); i != stats.end (); ++i, count++){

    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    if (t.destinationAddress == p2pdeviceIP.GetAddress(0)){
		throughput = (i->second.txBytes * 8) / ((i->second.timeLastTxPacket - i->second.timeFirstTxPacket).GetSeconds());
		throughput = ((throughput > 0) ? throughput : 0);
		sumThroughput += throughput;
		delay = i->second.delaySum.GetSeconds()/i->second.rxPackets;
		delay = ((delay > 0) ? delay : 0);
		meanDelayPackets += delay;
		meanLostPackets += i->second.lostPackets/((i->second.timeLastTxPacket - i->second.timeFirstTxPacket).GetSeconds());
		
		if(t.sourceAddress == devicesIP.GetAddress(nearestNode) || t.sourceAddress == devicesIP.GetAddress(farthestNode)){
			std::stringstream ss;
			ss << "./respository/Datasets/";
			//Nó de origem mais próximo
			if(devicesIP.GetAddress(nearestNode) == t.sourceAddress){
				ss <<prefix<<"_"<< "MP";
			}
			//Nó de origem mais distante
			else if (devicesIP.GetAddress(farthestNode) == t.sourceAddress){
				ss <<prefix<<"_"<< "ML";
			}
		
			switch(transportMode){
					case 0:
						if(traffic){
							ss <<"_UDP_cbr.csv";
						}
						else{
							ss <<"_UDP_burst.csv";
						}
						break;
					case 1:
						if(traffic){
							ss <<"_TCP_cbr.csv";
						}
						else{
							ss <<"_TCP_burst.csv";
						}
						break;
					case 2:
						if(traffic){
							ss <<"_MISTO_cbr.csv";
						}
						else{
							ss <<"_MISTO_burst.csv";
						}
						break;
			}
			f = fopen(ss.str().c_str(), "a");
			fprintf(f, "%d;%.2f;%.2f;%.2f;%d\n", nNodes, nearestNodeDistance, throughput/1024, delay, i->second.lostPackets);
			fclose(f);
		}
	}
  }
  
  meanThroughput  = sumThroughput / nNodes;
  meanDelayPackets /= nNodes;
  meanLostPackets /= nNodes;

  cout << "Throughput (mean)   : " << meanThroughput/1024 << " kbps"<< endl;
  cout << "Delay Packets (mean): " << meanDelayPackets << endl;
  cout << "Lost Packets (mean) : " << meanLostPackets << endl;

  std::stringstream ss;
  ss << "./respository/Datasets/";
  
  // if UDP/TCP/MISTO
    switch(transportMode){
		case 0:
			if(traffic){
				ss <<prefix<<"_"<<"UDP_cbr.csv";
			}
			else{
				ss <<prefix<<"_"<<"UDP_burst.csv";
			}
			break;
		case 1:
			if(traffic){
				ss <<prefix<<"_"<<"TCP_cbr.csv";
			}
			else{
				ss <<prefix<<"_"<<"TCP_burst.csv";
			}
			break;
		case 2:
			if(traffic){
				ss <<prefix<<"_"<<"MISTO_cbr.csv";
			}
			else{
				ss <<prefix<<"_"<<"MISTO_burst.csv";
			}
			break;
	}
	f = fopen(ss.str().c_str(), "a");
    fprintf(f, "%d;%.2f;%.2f;%.2f;%.2f\n", nNodes, meanThroughput/1024, meanDelayPackets, meanLostPackets, sumThroughput);
	fclose(f);
}

void run (){

  NodeContainer apnode, nodes, p2pnode;
  NetDeviceContainer apdevice, devices, p2pdevice;
  Ipv4InterfaceContainer apdeviceIP, devicesIP, p2pdeviceIP;

  //Create nodes
  p2pnode.Create (2);
  apnode = p2pnode.Get(1);
  nodes.Create (nNodes);

  // if value TRUE, exist mobility, else, not exist mobility.
  setMobility(apnode, nodes);

  installP2PDevices(p2pnode, p2pdevice);

  installWirelesp2pdevice(apnode, nodes, apdevice, devices);

  installInternetProtocol(apnode, nodes, p2pnode, apdevice, devices, p2pdevice, apdeviceIP, devicesIP, p2pdeviceIP);

  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // if udp/tcp/mixed mode
  switch(transportMode){
	  case 0:
		installUDPCommunication(nodes, p2pdeviceIP);
		break;
	  case 1:
		installTCPCommunication(nodes, p2pnode, p2pdeviceIP);
		break;
	  case 2:
		installUDPCommunication(nodes, p2pdeviceIP);
		installTCPCommunication(nodes, p2pnode, p2pdeviceIP);
		break;
  }

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // Simulation running time
  Simulator::Stop (Seconds (runningTime+2));
  Simulator::Run ();

  // execute magic of the flowmon
  monitor->SerializeToXmlFile("wifiinfra.xml", true, true);
  buildStatistics(flowmon, monitor, devicesIP, p2pdeviceIP);

  // Cleanup
  Simulator::Destroy ();
}

int main (int argc, char **argv){

  CommandLine cmd;

  cmd.AddValue("nodes", "Number of sta nodes", nNodes);
  cmd.AddValue("runningTime", "Application running time in seconds", runningTime);
  cmd.AddValue("traffic", "Traffic (CBR=true, Burst=false)", traffic);
  cmd.AddValue("transportMode", "Transport Mode (0=UDP, 1=TCP, 2=50%UDP + 50%TCP)", transportMode);
  cmd.AddValue("printLog", "Print Statistics? (true/false)", printLog);
  cmd.AddValue("prefix", "Name prefix of the file ", prefix);

  cmd.Parse (argc, argv);
  
   if(traffic){
    packetsize = 512;
    onTime = 0.01;
    offTime = 0.01;
  }
  else{
    packetsize = 1500;
    onTime = 3.0;
    offTime = 2.0;
  }
  
  run ();

  return 0;
}