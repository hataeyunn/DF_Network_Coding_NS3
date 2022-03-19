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

    uint32_t m_RelayNode = 9;
    uint32_t m_client = 12;
    std::map<uint32_t, Ipv4Address>                 idAndIp;
    std::map<uint32_t, std::vector<uint32_t>>    nodeConnections;
    std::vector<Ipv4Address>                        peersAddress;
    std::map<uint32_t, Ipv4Address>                 peerId;

    int         totalReceivedByte = 0;
    double      throughput = 0;
    double      totalLateny = 0;
    double      averageLatency = 0;
    int         sumCoding = 0;
    int         relatedK = 10;
    int         rate = 500;
    double      alpha = 1;
    double      dLimit = 10;
    int         totalSending = 0;
    int         totalReceiving = 0;
    std::string name1 = "coding.txt";
    std::string name2 = "dev.txt";

    
    CommandLine cmd;
    cmd.AddValue("relatedK", "Parameter for rleax delay",relatedK);
    cmd.AddValue("alpha", "Parameter for rleax delay 2",alpha);
    cmd.AddValue("datarate", "Parameter for data rate",rate);
    cmd.AddValue("dLimit", "Parameter for limit",dLimit);

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
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", 
        "DataMode", StringValue("HtMcs7"), "ControlMode", StringValue("HtMcs0"));
    
    mac.SetType("ns3::AdhocWifiMac");

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
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(9)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(11)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(13)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(15)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(17)->GetId());
    nodeConnections[allNodes.Get(0)->GetId()].push_back(allNodes.Get(19)->GetId());

    nodeConnections[allNodes.Get(1)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(1)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(1)->GetId()].push_back(allNodes.Get(4)->GetId());
             
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(1)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(10)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(12)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(14)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(16)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(18)->GetId());
    nodeConnections[allNodes.Get(2)->GetId()].push_back(allNodes.Get(20)->GetId());
    
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(4)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(6)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(13)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(15)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(11)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(17)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(9)->GetId());
    nodeConnections[allNodes.Get(3)->GetId()].push_back(allNodes.Get(19)->GetId());
    
    nodeConnections[allNodes.Get(4)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(4)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(4)->GetId()].push_back(allNodes.Get(1)->GetId());
    nodeConnections[allNodes.Get(4)->GetId()].push_back(allNodes.Get(7)->GetId());

    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(4)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(8)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(14)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(16)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(12)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(18)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(10)->GetId());
    nodeConnections[allNodes.Get(5)->GetId()].push_back(allNodes.Get(20)->GetId());
    
    nodeConnections[allNodes.Get(9)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(9)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(9)->GetId()].push_back(allNodes.Get(6)->GetId());

    nodeConnections[allNodes.Get(10)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(10)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(10)->GetId()].push_back(allNodes.Get(8)->GetId());

    nodeConnections[allNodes.Get(11)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(11)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(11)->GetId()].push_back(allNodes.Get(6)->GetId());

    nodeConnections[allNodes.Get(12)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(12)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(12)->GetId()].push_back(allNodes.Get(8)->GetId());

    nodeConnections[allNodes.Get(13)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(13)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(13)->GetId()].push_back(allNodes.Get(6)->GetId());

    nodeConnections[allNodes.Get(14)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(14)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(14)->GetId()].push_back(allNodes.Get(8)->GetId());

    nodeConnections[allNodes.Get(15)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(15)->GetId()].push_back(allNodes.Get(0)->GetId());
    nodeConnections[allNodes.Get(15)->GetId()].push_back(allNodes.Get(6)->GetId());

    nodeConnections[allNodes.Get(16)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(16)->GetId()].push_back(allNodes.Get(2)->GetId());
    nodeConnections[allNodes.Get(16)->GetId()].push_back(allNodes.Get(8)->GetId());

    nodeConnections[allNodes.Get(17)->GetId()].push_back(allNodes.Get(6)->GetId());
    nodeConnections[allNodes.Get(17)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(17)->GetId()].push_back(allNodes.Get(0)->GetId());
    
    nodeConnections[allNodes.Get(18)->GetId()].push_back(allNodes.Get(8)->GetId());
    nodeConnections[allNodes.Get(18)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(18)->GetId()].push_back(allNodes.Get(2)->GetId());
    
    nodeConnections[allNodes.Get(19)->GetId()].push_back(allNodes.Get(6)->GetId());
    nodeConnections[allNodes.Get(19)->GetId()].push_back(allNodes.Get(3)->GetId());
    nodeConnections[allNodes.Get(19)->GetId()].push_back(allNodes.Get(0)->GetId());
    
    nodeConnections[allNodes.Get(20)->GetId()].push_back(allNodes.Get(8)->GetId());
    nodeConnections[allNodes.Get(20)->GetId()].push_back(allNodes.Get(5)->GetId());
    nodeConnections[allNodes.Get(20)->GetId()].push_back(allNodes.Get(2)->GetId());
    

    NS_LOG_INFO("6. Set position");
    std::cout<<"6. Set position"<<"\n";

    MobilityHelper mobility;
    
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    
    positionAlloc->Add(Vector(5,15,0));    //0
    positionAlloc->Add(Vector(10,15,0));    //1
    positionAlloc->Add(Vector(15,15,0));    //2
    positionAlloc->Add(Vector(5,10,0));    //3
    positionAlloc->Add(Vector(10,10,0));    //4        
    positionAlloc->Add(Vector(15,10,0));    //5
    positionAlloc->Add(Vector(5,5,0));    //6
    positionAlloc->Add(Vector(10,5,0));    //7
    positionAlloc->Add(Vector(15,5,0));    //8

    positionAlloc->Add(Vector(0,15,0));     //9
    positionAlloc->Add(Vector(20,15,0));     //10
    positionAlloc->Add(Vector(0,13,0));     //11
    positionAlloc->Add(Vector(20,13,0));     //12
    positionAlloc->Add(Vector(0,10,0));     //13
    positionAlloc->Add(Vector(20,10,0));      //14
    positionAlloc->Add(Vector(0,8,0));    //15    
    positionAlloc->Add(Vector(20,8,0));    //16
    positionAlloc->Add(Vector(0,5,0));    //17    
    positionAlloc->Add(Vector(20,5,0));    //18
    positionAlloc->Add(Vector(0,3,0));    //19
    positionAlloc->Add(Vector(20,3,0));     //20

    


    mobility.SetPositionAllocator(positionAlloc);
    

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(allNodes);
    
    NS_LOG_INFO("7. Install Application");
    std::cout<<"7. Install Application"<<"\n";

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
            std::cout<<i<<","<<targetPeerIds[j]<<std::endl;
            peersAddress.push_back(idAndIp[targetPeerIds[j]]);
            peerId[targetPeerIds[j]] = idAndIp[targetPeerIds[j]];

        }
        df.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
        df.SetPeersAddresses(peersAddress);
        df.SetPeersIdAddresses(peerId);

        // Decide Mode (tre : DF, false: DSR)
        df.SetDelayForwardMode(true);
        if(targetId < 9 )
        {
            df.SetRelayMode(true);

        }
        else
        {
            df.SetRelayMode(false);
        }
        
        dfApp.Add(df.Install(targetNode));

        
    }

    
    dfApp.Start(Seconds(0.5));
    dfApp.Stop(Seconds(120));

    NS_LOG_INFO("Run Simulation");
    std::cout<<"Run simulation"<<"\n";

    
    phy.EnablePcap("DR_test5", devices);
    
    Simulator::Stop(Seconds(121));
    Simulator::Run();
    time_t curr_time;
    struct tm *curr_tm;
    curr_time = time(NULL);
    curr_tm = localtime(&curr_time);
    
    std::string path = "./result/"+std::to_string(curr_tm->tm_year)+std::to_string(curr_tm->tm_mon)+std::to_string(curr_tm->tm_mday)+"_"+std::to_string(curr_tm->tm_hour)+"_"+std::to_string(curr_tm->tm_min);
    mkdir(path.c_str(),0776);
    std::string filename = "relatedK"+std::to_string(relatedK)+"_"+"alpha"+std::to_string(alpha)+"_"+"rate"+std::to_string(rate)+".csv";
    std::ofstream outputFile1(name1,std::ios::app);
    std::ofstream outputFile2(name2,std::ios::app);

    outputFile1<<filename<<std::endl;
    outputFile2<<filename<<std::endl;

     for(uint32_t i = 0; i < allNodes.GetN(); ++i){
        Ptr<Node> targetNode = allNodes.Get(i);
        int receivedByte = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetReceivedByte();
        double thor = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetThroughput();
        double latency = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetLatency();
        int numCoding = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetNumberOfCoding();
        int numSending = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetNumberOfSending();
        int numReceiving = targetNode->GetApplication(0)->GetObject<DelayForward>()->GetNumberOfReceiving();

        totalReceivedByte += receivedByte;
        totalLateny += latency;
        sumCoding += numCoding;
        totalSending += numSending;
        totalReceiving += numReceiving; 

        std::cout<<"Node "<<targetNode->GetId()<<"'s totalreceivedByte = " << receivedByte <<"\n";
        std::cout<<"Node "<<targetNode->GetId()<<"'s throughput = " << thor<<"\n";
        std::cout<<"Node "<<targetNode->GetId()<<"'s latency = " << latency<<"\n";
        std::cout<<"Node "<<targetNode->GetId()<<"'s Coding  = " << numCoding<<"\n";
        outputFile1<<numCoding<<std::endl;
        outputFile2<<latency<<std::endl;
    }


    throughput = totalReceivedByte/(Simulator::Now().GetSeconds());
    averageLatency = totalLateny/allNodes.GetN();
    std::cout<<"Throughput : " << throughput <<"\n";
    std::cout<<"Latency : " << averageLatency <<"\n";
    std::cout<<"Coding : " << sumCoding <<"\n";
    std::cout<<"Number of Sending : " << totalSending << "\n";
    std::cout<<"Number of Receiving : " << totalReceiving << "\n";
    outputFile1.close();
    outputFile2.close();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
    
    
    
}
