/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
 *
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
 */

// CBR aplikacija => realizovana pomocu OnOffApplication
//                   FlowMonoitor radi i moze tracing
// Transportni moze da se menja: UDP (default) ili TCP
// AODV rutiranje
// Propagation Loss - matrix 

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/propagation-module.h"
#include "ns3/flow-monitor-helper.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "myapp.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("wormhole-AODV-ETX");

void
PacketSinkTraceSink (Ptr<const Packet> packet, const Address &from)
{
 // std::cout << "Packet received; UID = " << packet->GetUid () << std::endl;
}

void
PacketSinkTraceSinkContext (std::string context, Ptr<const Packet> packet, const Address &from)
{
  //std::cout << "Context = " << context << "; Packet received; UID = " << packet->GetUid () << std::endl;
}


int 
main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate2Mbps");
  std::string dataRate = "512Kbps";
  double txpDistance = 100;  // m
  uint32_t packetSize = 128; // bytes
  uint32_t numPackets = 10;
  double interval = 0.5; // seconds
  bool verbose = false;
  int x = 0;
  int j = 10;
  uint32_t nNodes = 8;
  std::string protocol = "ns3::UdpSocketFactory"; 
  uint16_t port = 80;
  float ThroughputNetDevice = 0; // Default Value Throughput NetDevice
  double N = 100.0;

  //LogComponentEnable ("AodvRoutingProtocol", LOG_LEVEL_DEBUG);

  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("txpDistance", "Specify node's transmit range [m], Default:100", txpDistance);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("transport TypeId", "TypeId for socket factory", protocol);
  cmd.AddValue ("dataRate", "Number of Data Rate", dataRate);

  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);


  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      StringValue (phyMode));


   NodeContainer c;
   NodeContainer not_malicious;

   NodeContainer malicious;

   c.Create (nNodes);
   not_malicious.Add(c.Get(0));
   not_malicious.Add(c.Get(3));
   not_malicious.Add(c.Get(4));
   not_malicious.Add(c.Get(5));
   not_malicious.Add(c.Get(1));
   not_malicious.Add(c.Get(7));
   malicious.Add(c.Get(6));
   malicious.Add(c.Get(2));


  // Note that with RangePropagationLossModel, the positions below are not 
  // used for received signal strength. 
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  /*
  positionAlloc->Add (Vector (0, 10, 3.0));    //0
  positionAlloc->Add (Vector (50, 10, 3.0));   //1
  positionAlloc->Add (Vector (100, 10, 3.0));   //2
  positionAlloc->Add (Vector (150, 10, 3.0));  //3
  positionAlloc->Add (Vector (0, 60, 3.0));   //4
  positionAlloc->Add (Vector (50, 60, 3.0));  //5
  positionAlloc->Add (Vector (100, 60, 3.0));  //6
  positionAlloc->Add (Vector (150, 60, 3.0)); //7
  */
  for (x=0;x<5;x++){
    positionAlloc->Add (Vector (0, j, 3.0));    //0
    positionAlloc->Add (Vector (50, j, 3.0));   //1
    positionAlloc->Add (Vector (100, j, 3.0));   //2
    positionAlloc->Add (Vector (150, j, 3.0));  //3
    //positionAlloc->Add (Vector (200, j, 3.0));  //3
    j = j + 30;
  }
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

  // Create propagation loss matrix
  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (200); // set default loss to 200 dB (no link)
  
  lossModel->SetLoss (c.Get (0)->GetObject<MobilityModel>(), c.Get (1)->GetObject<MobilityModel>(), 111); // loss 0 <-> 1
  lossModel->SetLoss (c.Get (0)->GetObject<MobilityModel>(), c.Get (4)->GetObject<MobilityModel>(), 111); // loss 0 <-> 4
  lossModel->SetLoss (c.Get (0)->GetObject<MobilityModel>(), c.Get (5)->GetObject<MobilityModel>(), 40); // loss 0 <-> 5

  lossModel->SetLoss (c.Get (1)->GetObject<MobilityModel>(), c.Get (2)->GetObject<MobilityModel>(), 111); // loss 1 <-> 2
  lossModel->SetLoss (c.Get (1)->GetObject<MobilityModel>(), c.Get (4)->GetObject<MobilityModel>(), 111); // loss 1 <-> 4
  lossModel->SetLoss (c.Get (1)->GetObject<MobilityModel>(), c.Get (5)->GetObject<MobilityModel>(), 111); // loss 1 <-> 5
  lossModel->SetLoss (c.Get (1)->GetObject<MobilityModel>(), c.Get (6)->GetObject<MobilityModel>(), 111); // loss 1 <-> 6
  
  lossModel->SetLoss (c.Get (2)->GetObject<MobilityModel>(), c.Get (3)->GetObject<MobilityModel>(), 111); // loss 2 <-> 3
  lossModel->SetLoss (c.Get (2)->GetObject<MobilityModel>(), c.Get (5)->GetObject<MobilityModel>(), 40); // loss 2 <-> 5
  lossModel->SetLoss (c.Get (2)->GetObject<MobilityModel>(), c.Get (6)->GetObject<MobilityModel>(), 111); // loss 2 <-> 6
  lossModel->SetLoss (c.Get (2)->GetObject<MobilityModel>(), c.Get (7)->GetObject<MobilityModel>(), 40); // loss 2 <-> 7

  lossModel->SetLoss (c.Get (3)->GetObject<MobilityModel>(), c.Get (6)->GetObject<MobilityModel>(), 111); // loss 3 <-> 6
  lossModel->SetLoss (c.Get (3)->GetObject<MobilityModel>(), c.Get (7)->GetObject<MobilityModel>(), 111); // loss 3 <-> 7

  lossModel->SetLoss (c.Get (4)->GetObject<MobilityModel>(), c.Get (5)->GetObject<MobilityModel>(), 111); // loss 4 <-> 5

  lossModel->SetLoss (c.Get (5)->GetObject<MobilityModel>(), c.Get (6)->GetObject<MobilityModel>(), 111); // loss 5 <-> 6

  lossModel->SetLoss (c.Get (6)->GetObject<MobilityModel>(), c.Get (7)->GetObject<MobilityModel>(), 111); // loss 6 <-> 7

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  // Create & setup wifi channel
  Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
  wifiChannel->SetPropagationLossModel (lossModel);
  wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
  wifiPhy.SetChannel (wifiChannel);


  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
  NetDeviceContainer mal_devices = wifi.Install(wifiPhy, wifiMac, malicious);


  InternetStackHelper internet;
  // AODV - set routing protocol !!!
  AodvHelper aodv;
  AodvHelper malicious_aodv;
  internet.SetRoutingHelper(aodv);
  internet.Install (not_malicious);

  malicious_aodv.Set("EnableWrmAttack",BooleanValue(true)); // putting *false* instead of *true* would disable the malicious behavior of the node
  malicious_aodv.Set("FirstEndWifiWormTunnel",Ipv4AddressValue("192.168.1.1"));
  malicious_aodv.Set("FirstEndWifiWormTunnel",Ipv4AddressValue("192.168.1.5"));



  internet.SetRoutingHelper (malicious_aodv);
  internet.Install (malicious);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  ipv4.SetBase ("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer mal_ifcont = ipv4.Assign (mal_devices);

  // Soket
  TypeId tid = TypeId::LookupByName (protocol);
  InetSocketAddress destinationAddress = InetSocketAddress (Ipv4Address ("192.168.1.1"), port);
  InetSocketAddress sinkAddress = InetSocketAddress (Ipv4Address::GetAny (), port); 

  // Applications
  OnOffHelper onOff (protocol, destinationAddress);
  onOff.SetConstantRate (DataRate (dataRate), packetSize);
  ApplicationContainer serverApps = onOff.Install (c.Get (7));
  serverApps.Start (Seconds (15.0));
  serverApps.Stop (Seconds (N-15));

  PacketSinkHelper sink (protocol, sinkAddress);
  ApplicationContainer clientApps = sink.Install (c.Get (0));
  clientApps.Start (Seconds (15.0));
  clientApps.Stop (Seconds (N-10));

  // Tracing
  Config::ConnectWithoutContext ("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx", MakeCallback (&PacketSinkTraceSink));
  Config::Connect ("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx", MakeCallback (&PacketSinkTraceSinkContext));
  wifiPhy.EnablePcap ("etf-floor", devices);
  
  // Output what we are doing
  NS_LOG_UNCOND ("Testing ...");

  // Flow monitor
  FlowMonitorHelper flowMonHlp;
  Ptr <FlowMonitor> flowMon = flowMonHlp.InstallAll ();

  // NetAnim simulator
  AnimationInterface anim ("netanim.xml");
  //anim.EnablePacketMetadata (true);
  anim.SetMobilityPollInterval (Seconds (1));
  anim.EnableIpv4RouteTracking("aodv-routing.xml",Time(Seconds(20)), Time(Seconds(N-10)), c);
  anim.AddSourceDestination(7, "192.168.1.1");

	

  // Output config store to txt/xml format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  //Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("etf-floor-attributes.xml"));
  //Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();  
  
  Simulator::Stop (Seconds (N));
  Simulator::Run ();
  
 // Show FlowflowMon
  std::cout << std::endl <<" ( Flow flowMon statistics - AODV-ETX-WORMHOLE "<< packetSize << " B )" << std::endl;
  flowMon->CheckForLostPackets();
  ns3::Ipv4FlowClassifier *classifier = dynamic_cast<Ipv4FlowClassifier*>(&(*(flowMonHlp.GetClassifier ())));
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMon->GetFlowStats();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    if ((t.sourceAddress=="192.168.1.8" && t.destinationAddress == "192.168.1.1"))
      {
          std::cout << "  Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024  << "Kbps\n";
          std::cout << "  End-to-End Delay:   " << i->second.delaySum << "\n";
          std::cout << "  Tx Packets:   " << i->second.txPackets << std::endl;
          std::cout << "  Rx Packets:   " << i->second.rxPackets << std::endl;
          std::cout << "  Packets Lost:   " << i->second.lostPackets << std::endl;
          std::cout << "  Packets Lost (%):   " << ((i->second.txPackets-i->second.rxPackets)*1.0)/i->second.txPackets << std::endl;
     }
    }

  flowMon -> SerializeToXmlFile ("flomon.xml", true, true);
  Simulator::Destroy ();

  return 0;
}
