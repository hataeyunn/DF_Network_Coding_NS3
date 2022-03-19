#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <ctime>
#include <map>
#include <unistd.h>
#include "sys/types.h"
#include "sys/stat.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DF-test-r5");

int main(int argc, char *argv[])
{

    uint32_t m_RelayNode = 4;
    uint32_t m_client = 2;
    std::map<uint32_t, Ipv4Address>                 idAndIp;
    std::map<uint32_t, std::vector<uint32_t>>    nodeConnections;
    std::vector<Ipv4Address>                        peersAddress;
    std::map<uint32_t, Ipv4Address>                 peerId;

    //int         totalReceivedByte = 0;
    double      totalthroughput = 0;
    //double      throughput = 0;
    double      totalLateny = 0;
    //double      averageLatency = 0;
    int         sumCoding = 0;
    int         relatedK = 3;
    int         rate = 1;
    double      alpha = 0.5;
    double      dLimit = 0.01;
    int         totalSending = 0;
    int         totalReceiving = 0;
    bool        isdf = false;
    
    CommandLine cmd;
    cmd.AddValue("relatedK", "Parameter for rleax delay",relatedK);
    cmd.AddValue("alpha", "Parameter for rleax delay 2",alpha);
    cmd.AddValue("datarate", "Parameter for data rate",rate);
    cmd.AddValue("dLimit", "Parameter for limit",dLimit);
    cmd.AddValue("isdf","Parameter for df coding",isdf);
    cmd.Parse(argc, argv);
    

    peersAddress.clear();
    peerId.clear();
    nodeConnections.clear();

    NS_LOG_INFO("1. Create Nodes");
    std::cout<<"1. Create Nodes"<<"\n";

    NodeContainer relayNodes;
    NodeContainer clientNodes;

    relayNodes.Create(m_RelayNode);
    clientNodes.Create(m_client);

    NodeContainer allNodes = NodeContainer(relayNodes, clientNodes);

    NS_LOG_INFO("2. Create PHY Layer");
    std::cout<<"2. Create PHY Layer"<<"\n";

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    phy.SetChannel(channel.Create());
    phy.Set("ShortGuardEnabled", BooleanValue(false));
    
    NS_LOG_INFO("3. Create WALN setting");
    std::cout<<"3. Create WALN setting"<<"\n";

    WifiMacHelper mac;
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211ac);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("OfdmRate6Mbps"));
    
    mac.SetType("ns3::AdhocWifiMac");
    
/*
    //-------------------------
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
    NetDeviceContainer devices;
    devices = wifi.Install (wifiPhy, wifiMac, allNodes);
    // setup ap.

    //-------------------------
*/ 
    NS_LOG_INFO("4. Create Netdevices");
    std::cout<<"4. Create  Netdevices"<<"\n";

    NetDeviceContainer devices;
    devices = wifi.Install(phy, mac, allNodes);


    NS_LOG_INFO("5. Assign IP Addresses");
    std::cout<<"5. Assign IP Addresses"<<"\n";

    InternetStackHelper stack;
    stack.Install(allNodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0","255.255.255.0");

    //Ipv4InterfaceContainer inter = ipv4.Assign(devices);
    Ipv4InterfaceContainer inter;

    for(uint32_t i = 0; i < allNodes.GetN();++i)
    {
        inter.Add(ipv4.Assign(devices.Get(i)));
    }
    
    for(uint32_t i = 0; i < allNodes.GetN();++i)
    {
        uint32_t nodeId = allNodes.Get(i)->GetId();
        //std::cout << "Node id : " <<nodeId << "\n";
        //Address nodeAddress = allNodes.Get(i)->GetDevice(0)->GetAddress();
        auto nodeAddress = inter.GetAddress(i);
        //std::cout << "Node IP : " <<nodeAddress << "\n";
        //idAndIp[nodeId] = InetSocketAddress::ConvertFrom(nodeAddress).GetIpv4();   
        idAndIp[nodeId] = nodeAddress;
        std::cout<<"Node "<< nodeId <<"'s ip is " << idAndIp[nodeId] <<"\n";
    }



    
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(1)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(4)->GetId());

    nodeConnections[allNodes.Get(1)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(1)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(1)->GetId()].push_back(allNodes.Get(5)->GetId());

    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(4)->GetId());
     
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(1)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(5)->GetId());


    nodeConnections[allNodes.Get(4)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(4)->GetId()].push_back(allNodes.Get(2)->GetId());

    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(1)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(3)->GetId());

    NS_LOG_INFO("6. Set position");
    std::cout<<"6. Set position"<<"\n";

    MobilityHelper mobility;
    
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    
    positionAlloc->Add(Vector(10,30,0));    //0
    positionAlloc->Add(Vector(30,30,0));    //1

    positionAlloc->Add(Vector(10,10,0));    //2
    positionAlloc->Add(Vector(30,10,0));    //3

    positionAlloc->Add(Vector(4,20,0));    //4
    positionAlloc->Add(Vector(36,20,0));    //5

    mobility.SetPositionAllocator(positionAlloc);
    

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(allNodes);
    AnimationInterface anim("topology.xml");
    NS_LOG_INFO("7. Install Application");
    std::cout<<"7. Install Applicatio"<<"\n";

    uint16_t port = 8080;
    Address localAddress(InetSocketAddress (Ipv4Address::GetAny(), port));

    DelayForwardHelper df(true, localAddress, true, true, allNodes.GetN(), relatedK, alpha, rate, dLimit, peersAddress, peerId);

    //df.SetPeersAddresses(peerAddress);
    //df.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));

    ApplicationContainer dfApp;
    
    for(uint32_t i = 0; i < allNodes.GetN(); ++i)
    {
        Ptr<Node> targetNode = allNodes.Get(i);
        uint32_t targetId = targetNode->GetId();
        std::vector<uint32_t> targetPeerIds = nodeConnections[targetId];


        peersAddress.clear();
        peerId.clear();

        for(uint32_t j = 0; j < targetPeerIds.size(); ++j)
        {
            peersAddress.push_back(idAndIp[targetPeerIds[j]]);
            peerId[targetPeerIds[j]] = idAndIp[targetPeerIds[j]];
        }
        
        df.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
        df.SetPeersAddresses(peersAddress);
        df.SetPeersIdAddresses(peerId);

        // Decide Mode (true : DF, false: DSR)
        df.SetDelayForwardMode(isdf);

        if(targetId < 4 )
        {
            df.SetRelayMode(true);
        }
        else
        {
            df.SetRelayMode(false);
        }
        
        dfApp.Add(df.Install(targetNode));

        
    }

    
    dfApp.Start(Seconds(0.2));
    dfApp.Stop(Seconds(700));

    NS_LOG_INFO("Run Simulation");
    std::cout<<"Run simulation"<<"\n";

    
    //phy.EnablePcap("DR_test5", devices);
    
    Simulator::Stop(Seconds(701));
    Simulator::Run();
    time_t curr_time;
    struct tm *curr_tm;
    curr_time = time(NULL);
    curr_tm = localtime(&curr_time);
    
    std::string path = "./result/"+std::to_string(curr_tm->tm_year)+std::to_string(curr_tm->tm_mon)+std::to_string(curr_tm->tm_mday)+"_"+std::to_string(curr_tm->tm_hour)+"_"+std::to_string(curr_tm->tm_min);
    mkdir(path.c_str(),0776);
    std::string filename = path+"/relatedK"+std::to_string(relatedK)+"_"+"alpha"+std::to_string(alpha)+"_"+"rate"+std::to_string(rate)+".csv";
    std::ofstream outputFile(filename);
    outputFile<<"node"<<","<<"throughput"<<","<<"latency"<<","<<"receivedByte"<<","<<"numCoding"<<std::endl;
     for(uint32_t i = 0; i < allNodes.GetN(); ++i){
        Ptr<Node> targetNode = allNodes.Get(i);
        int receivedByte = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetReceivedByte();
        double latency = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetLatency();

        double thor = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetThroughput();
        int numCoding = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetNumberOfCoding();
        int numSending = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetNumberOfSending();
        int numReceiving = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetNumberOfReceiving();

        //totalReceivedByte += receivedByte;
        //totalLateny += latency;
        sumCoding += numCoding;
        totalSending += numSending;
        totalReceiving += numReceiving;
        if(i>=m_RelayNode){
            totalLateny += latency;
            totalthroughput += thor;
        }
        std::cout<<"Node "<<targetNode->GetId()<<"'s totalreceivedByte = " << receivedByte <<"\n";
        std::cout<<"Node "<<targetNode->GetId()<<"'s throughput = " << thor<<"\n";
        std::cout<<"Node "<<targetNode->GetId()<<"'s latency = " << latency<<"\n";
        std::cout<<"Node "<<targetNode->GetId()<<"'s Coding  = " << numCoding<<"\n";
        outputFile<<targetNode->GetId()<<","<<thor<<","<<latency<<","<<receivedByte<<","<<numCoding<<std::endl;

    }
    outputFile.close();
    
    //averageLatency = totalLateny/allNodes.GetN();
    //throughput = totalReceivedByte/averageLatency;
    std::cout<<"Throughput : " << totalthroughput/m_client <<"\n";
    std::cout<<"Latency : " << totalLateny/m_client <<"\n";
    std::cout<<"Coding : " << sumCoding <<"\n";
    std::cout<<"Number of Sending : " << totalSending << "\n";
    std::cout<<"Number of Receiving : " << totalReceiving << "\n";

    Simulator::Destroy();
    NS_LOG_INFO("Done.");
    
    
    
}
