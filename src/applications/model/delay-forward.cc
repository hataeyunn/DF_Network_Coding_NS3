#include "ns3/log.h"
#include "delay-forward.h"
#include "ns3/udp-socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/config.h"
#include <utility>
#include <cmath>
#include <ctime>
#include <string>
namespace ns3{

    NS_LOG_COMPONENT_DEFINE("DelayForward");
    NS_OBJECT_ENSURE_REGISTERED(DelayForward);

    TypeId DelayForward::GetTypeId(void){

        static TypeId tid = TypeId("ns3::DelayForward")
            .SetParent<Application>()
            .AddConstructor<DelayForward>()
            .AddAttribute("Mode", "The mode: Normal(True), Multi-hop(true)",
                BooleanValue(true), MakeBooleanAccessor(&DelayForward::m_mode),
                MakeBooleanChecker())
            .AddAttribute("Local", "The address on which to bind the rx socket",
                AddressValue(),
                MakeAddressAccessor(&DelayForward::m_local),
                MakeAddressChecker())
            .AddAttribute("Protocol", "The type id of the protocol to use for the rx socket",
                TypeIdValue(UdpSocketFactory::GetTypeId()),
                MakeTypeIdAccessor(&DelayForward::m_tid),
                MakeTypeIdChecker())
            .AddAttribute("DataRate", "The data rate",
                DataRateValue(DataRate("500kb/s")),
                MakeDataRateAccessor(&DelayForward::m_dataRate),
                MakeDataRateChecker())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                MakeTraceSourceAccessor(&DelayForward::m_txTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx", "A packet has been received",
                MakeTraceSourceAccessor(&DelayForward::m_rxTrace),
                "ns3::Packet::TracedCallback");

        return tid;

    }
    
    DelayForward::DelayForward():
        m_IsRelayNode(false)
    {
        NS_LOG_FUNCTION(this);
        m_sequenceNumber = 1;
        m_totalReceivedByte = 0;
        m_throughput = 0;
        m_averageLantency = 0;
        m_dLimit = 0.1;
        m_bestDelay = 100;
        m_numberOfCoding = 0;
        m_queueTime = 0.015;
        m_numberOfSending = 0;
        m_numberOfReceiving = 0;
        m_sumLatency = 0;
        m_numberofbidir = 0;
    }

    DelayForward::~DelayForward()
    {
        NS_LOG_FUNCTION(this);
    }

    void
    DelayForward::StartApplication(void)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Node :" << GetNode()->GetId() <<" peers are");
        
        for(auto it = m_peersAddresses.begin(); it != m_peersAddresses.end(); it++)
        {
            NS_LOG_INFO("\t" << *it);
        }

        TypeId tid = m_tid;

        if (!m_socket)
        {
            NS_LOG_INFO("Node : " << GetNode()->GetId() << " creates socket");
            std::cout<<"Node : " << GetNode()->GetId() << " creates socket\n";
            InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 8080);
            //InetSocketAddress remote = InetSocketAddress (Ipv4Address::GetAny(), 80);
            
            m_socket = Socket::CreateSocket(GetNode(), tid);
            //m_socket->SetAllowBroadcast(true);
            m_socket->Connect(remote);
            m_socket->Bind(m_local);
            m_socket->Listen();
            //m_socket->ShutdownSend();

        }

        m_socket->SetRecvCallback(MakeCallback(&DelayForward::HandleRead, this));
        m_socket->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&DelayForward::HandleAccept, this));
        m_socket->SetCloseCallbacks(
            MakeCallback(&DelayForward::HandlePeerClose, this),
            MakeCallback(&DelayForward::HandlePeerError, this));


        if(m_peersAddresses.size() == 0)
        {
            NS_LOG_INFO("Node :" << GetNode()->GetId() <<" do not have peers");
        }
        else
        {
            for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end() ; ++i)
            {
                std::cout<<"Node :" << GetNode()->GetId() <<" will connect a peer " << *i<<std::endl;
                NS_LOG_INFO("Node :" << GetNode()->GetId() <<" will connect a peer " << *i);
                m_peersSockets[*i] = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
                m_peersSockets[*i]->Connect(InetSocketAddress(*i, 8080));
            }

            
            m_peersSockets[Ipv4Address ("255.255.255.255")] = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            m_peersSockets[Ipv4Address ("255.255.255.255")]->Connect(InetSocketAddress(Ipv4Address ("255.255.255.255"), 9));
            m_peersSockets[Ipv4Address ("255.255.255.255")]->Bind(m_local);
            //m_peersSockets[Ipv4Address ("255.255.255.255")]->SetAllowBroadcast(true);
            m_peersSockets[Ipv4Address ("255.255.255.255")]->ShutdownRecv();

            /*
            m_peersSockets[Ipv4Address::GetAny()] = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            m_peersSockets[Ipv4Address::GetAny()]->Listen();
            m_peersSockets[Ipv4Address::GetAny()]->ShutdownSend();

            
            m_peersSockets[Ipv4Address::GetAny()]->SetRecvCallback(MakeCallback(&DelayForward::HandleRead, this));
            m_peersSockets[Ipv4Address::GetAny()]->SetAcceptCallback(
                    MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
                    MakeCallback(&DelayForward::HandleAccept, this));
            m_peersSockets[Ipv4Address::GetAny()]->SetCloseCallbacks(
                    MakeCallback(&DelayForward::HandlePeerClose, this),
                    MakeCallback(&DelayForward::HandlePeerError, this));
            */
            
       
        }

        //std::cout<<"Node : " << GetNode()->GetId() <<  " df mode : " << m_IsDfMode << "\n";
        //std::cout<<"----------------------------------\n";
        //std::cout<<"Node " << GetNode()->GetId() << "'s peer table\n";
        for(auto it =  m_peersIdAddresses.begin() ; it != m_peersIdAddresses.end(); it++)
        {
            m_peersIds.push_back(it->first);
            //std::cout<<"Peer node id: " <<it->first<<",  ip : " << it->second << "\n";
        }

        //std::cout<<"----------------------------------\n";
        //std::cout<<"Node " << GetNode()->GetId() << "'s bidirectionality table\n";

        for(uint32_t i = 0 ; i < m_peersIds.size(); i++)
        {
            for(uint32_t j = 0; j < m_peersIds.size(); j++)
            {
                if(i != j)
                {
                    //std::pair<uint32_t, uint32_t>   twoHop = make_pair(m_peersIds[i], m_peersIds[j]);
                    m_bidirectionTable[std::pair<uint32_t, uint32_t>(m_peersIds[i], m_peersIds[j])] = 0; 
                    /*
                    std::cout<<"in id: " <<m_peersIds[i]<<",  out id : " << m_peersIds[j] 
                        << ",  bidir : "<< m_bidirectionTable[std::pair<uint32_t, uint32_t>(m_peersIds[i], m_peersIds[j])] <<"\n";
                    */
                }

            }

        }
        //std::cout<<"----------------------------------\n";

        /*
        if (GetNode()->GetId() == 0)
        {
            m_destinationNodeId = 3;
        } 
        else if(GetNode()->GetId() == 3)
        {
            m_destinationNodeId = 0;
        }
        else if(GetNode()->GetId() == 4)
        {
            m_destinationNodeId = 3;
        }
        

        if(!m_IsRelayNode)
        {
            ReadyToSendData();
        }
        */

        
        if(!m_IsRelayNode && GetNode()->GetId()%2 == 1)
        {
            m_destinationNodeId = (uint32_t)(GetNode()->GetId() - 1);
             
        }
        else if(!m_IsRelayNode && GetNode()->GetId()%2 != 1)       
        {
            m_destinationNodeId = (uint32_t)(GetNode()->GetId() + 1);
        }


        if(!m_IsRelayNode)
        {
            //std::cout<<"Node : "<<GetNode()->GetId() << ", dest : " << m_destinationNodeId << "\n";

            ReadyToSendData();
        }




    }

    void
    DelayForward::StopApplication(void)    
    {
        NS_LOG_FUNCTION(this);

        
        //std::cout<<"!!!Node "<< GetNode()->GetId() <<" application stop!!!\n";;
        //std::cout<<"Node "<< GetNode()->GetId() <<" shows flow table\n";;
        //for(std::vector<std::pair<uint32_t, uint32_t>>::const_iterator i = m_flowTable.begin(); i != m_flowTable.end() ; ++i)
        //{
        //    std::cout<<"in : " << i->first << ", out : " << i->second << "\n";
        //}

        //std::cout<<"----------------------------------\n";
        //std::cout<<"Node " << GetNode()->GetId() << "'s bidirectionality table\n";

        for(uint32_t i = 0 ; i < m_peersIds.size(); i++)
        {
            for(uint32_t j = 0; j < m_peersIds.size(); j++)
            {
                if(i != j)
                {
                    //std::pair<uint32_t, uint32_t>   twoHop = make_pair(m_peersIds[i], m_peersIds[j]);
                    //std::cout<<"in id: " <<m_peersIds[i]<<",  out id : " << m_peersIds[j] 
                    //    << ",  bidir : "<< m_bidirectionTable[std::pair<uint32_t, uint32_t>(m_peersIds[i], m_peersIds[j])] <<"\n";
                    m_numberofbidir+=m_bidirectionTable[std::pair<uint32_t, uint32_t>(m_peersIds[i], m_peersIds[j])];
                }

            }

        }
        std::cout<<"node : "<<GetNode()->GetId()<< " bidir : " << m_numberofbidir/4<<std::endl;

        
        if(!m_IsRelayNode)
        {
            std::cout<<"-------------route---------------------\n";
            
            for(std::vector<uint32_t>::const_iterator it = m_route.begin(); it != m_route.end() ; ++it)
            {
                std::cout<<*it <<" -> " ;
            }

            std::cout<< GetNode()->GetId() <<"\n";
            std::cout<<"----------------------------------\n";

        }
        

        if(m_dataEvent.IsRunning()){
            Simulator::Cancel(m_dataEvent);
        }

        if(m_dataEvent.IsRunning()){
            Simulator::Cancel(m_dataEvent);
        }

        for(std::vector<Ipv4Address>::iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            m_peersSockets[*i]->Close();
        }

        if(m_socket)
        {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }

    }

    void
    DelayForward::SetPeersAddresses(const std::vector<Ipv4Address> &peers)
    {
        NS_LOG_FUNCTION(this);
        m_peersAddresses = peers;
        m_numberOfPeers = m_peersAddresses.size();
        NS_LOG_INFO("Set peersAddresses (size: " <<  m_peersAddresses.size() << ")" );

    }
    
    void
    DelayForward::SetPeersIdAddresses(const std::map<uint32_t, Ipv4Address> &peersIds)
    {
        NS_LOG_FUNCTION(this);
        m_peersIdAddresses = peersIds;
        
    }

    void
    DelayForward::SetDelayForwardMode(bool dfMode)
    {
        NS_LOG_FUNCTION(this);
        m_IsDfMode = dfMode;
    }

    void
    DelayForward::SetRelayMode(bool relayMode)
    {
        NS_LOG_FUNCTION(this);
        m_IsRelayNode = relayMode;
    }
    
    void
    DelayForward::SetNumberOfNodes(int numberOfNodes)
    {
        NS_LOG_FUNCTION(this);
        m_numberOfNodes = numberOfNodes;

    }

    void
    DelayForward::SetRelaxedK(int relaxedK)
    {
        NS_LOG_FUNCTION(this);
        m_relaxedK = relaxedK;
    }

    void
    DelayForward::SetRelaxedAlpha(double relaxedAlpha)
    {
        NS_LOG_FUNCTION(this);
        m_relaxedAlpha = relaxedAlpha;
        
    }

    void
    DelayForward::SetRate(int rate)
    {
        NS_LOG_FUNCTION(this);
        m_rate = rate;

    }
    
    void
    DelayForward::SetDLimit(double dLimt)
    {
        NS_LOG_FUNCTION(this);
        m_dLimit = dLimt;
    }

    void
    DelayForward::ReadyToSendData(void)
    {
        srand((unsigned int)time(NULL));

        NS_LOG_FUNCTION(this);
        //Time tNext(Seconds(1));
        //uint32_t    nodeId = GetNode()->GetId();
        //Time tNext(Seconds(1));
        double tenSec = 10.0;
        double tr = (rand()%5+1)/tenSec;
        //std::cout<< "tr : " << tr <<"\n";
        Time tNext = Seconds(tr);

        m_copiedRate = m_rate;

        if(GetNode()->GetId()<70){
        if(!HasFlow(m_destinationNodeId))
        {
            
            
            tNext = Seconds((double)GetNode()->GetId());
            //std::cout<< "At time " << Simulator::Now().GetSeconds() << " Node "<<GetNode()->GetId()<< " : RREQ will be sent \n";
            m_dataEvent = Simulator::Schedule(tNext, &DelayForward::SendRREQ, this);
        }
        else
        {
            //std::cout<<"Node "<<GetNode()->GetId()<< " wait to send data during "<< tNext <<"\n";
            m_dataEvent = Simulator::Schedule(tNext, &DelayForward::SendData, this);
            //std::cout<<tNext<<std::endl;            
        }  
        }

        
    }

    bool
    DelayForward::HasFlow(uint32_t destId)
    {
        NS_LOG_FUNCTION(this);
        if(m_routingTable.find(destId) != m_routingTable.end())
        {
            return true;
        }
        else
        {
            return false;
        }
        
    }

    bool
    DelayForward::HasSequenceNumbner(uint32_t source, int sequence)
    {
        NS_LOG_FUNCTION(this);

        if(m_sequenceTable.find(source) != m_sequenceTable.end())
        {
            if(m_sequenceTable[source] >= sequence)
            {
                return true;
            }
            else
            {
                //m_sequenceTable[source] = sequence;
                return false;
            } 
        }
        else
        {
            //m_sequenceTable[source] = sequence;
            return false;
        }
        
    }


    void
    DelayForward::SetBidiractional(void)
    {
        NS_LOG_FUNCTION(this);

        //std::vector<std::pair<uint32_t, uint32_t>>  tempTable;
        int bidirect;
        uint32_t    inNode;
        uint32_t    outNode;
        uint32_t    jInNode;
        uint32_t    jOutNode;

        if(m_bidirectionTable.size())
        {
            for(std::vector<std::pair<uint32_t, uint32_t>>::const_iterator i = m_flowTable.begin(); i != m_flowTable.end() ; ++i)
            {
                bidirect = 0;
                inNode = i->first;
                outNode = i->second;

                for(std::vector<std::pair<uint32_t, uint32_t>>::const_iterator j = m_flowTable.begin(); j != m_flowTable.end() ; ++j)
                {
                    jInNode = j->first;
                    jOutNode = j->second;

                    if((inNode == jInNode && outNode == jOutNode) || (inNode == jOutNode && outNode == jInNode))
                    {
                        bidirect++;
                    }
                }
                m_bidirectionTable[*i] = bidirect;
            }   
        }




    }

    int
    DelayForward::GetBidirectional(uint32_t inNode, uint32_t outNode)
    {
        NS_LOG_FUNCTION(this);

        //std::pair<uint32_t, uint32_t>(m_peersIds[i], m_peersIds[j])
        
        for(auto it = m_bidirectionTable.begin(); it != m_bidirectionTable.end(); it++)
        {
            if(it->first == std::pair<uint32_t, uint32_t>(inNode, outNode))
            {
                return it->second;
            }
        }

        return 0;
    }

    uint32_t
    DelayForward::GetNodeIdFromIP(Ipv4Address ip)
    {
       NS_LOG_FUNCTION(this);

       for(auto it = m_peersIdAddresses.begin(); it != m_peersIdAddresses.end(); it++)
        {
            if(it->second == ip)
            {
                return it->first;
            }
        }

        return 0;
    }

    double
    DelayForward::GetDelayTime(int bidir)
    {
        NS_LOG_FUNCTION(this);

        double delaytime=0;
        std::cout<<bidir<<std::endl;
        if(m_relaxedAlpha == 0)
        {
            delaytime = 0;
            //std::cout<<"[Delay Time] Node " << GetNode()->GetId() << " : bidir = " << bidir << " , delaytime =  " <<  delaytime <<"\n";
            return delaytime;
        }

        if(bidir < m_relaxedK*2)
        {
            bidir = bidir/2;

            delaytime = (1/pow(m_relaxedAlpha, bidir))*(1/pow((m_numberOfNodes-1), (bidir+1)))*m_dLimit;
            //std::cout<<"[Delay Time] Node " << GetNode()->GetId() << " : bidir = " << bidir << " , delaytime =  " <<  delaytime <<"\n";

        }
        else
        {
            if(m_relaxedK != 0)
            {
                delaytime = 10;
                //std::cout<<"[Delay Time] Node " << GetNode()->GetId() << " : bidir = " << bidir << " , delaytime =  " <<  delaytime <<"\n";

                //std::cout<<delaytime<<std::endl;

            }
            else
            {
                /* code */
                delaytime = (1/pow((m_numberOfNodes-1), (m_relaxedK+1)))*m_dLimit;
            }
                        
        }
        //std::cout<<"[Delay Time] Node " << GetNode()->GetId() << " : bidir = " << bidir << " , delaytime =  " <<  delaytime <<"\n";
        //std::cout<<"m_numberofnodes = " << m_numberOfNodes << " , delaytime =  " <<  delaytime <<"\n";

        return delaytime;

    }

    bool
    DelayForward::HasNetworkCoding(uint32_t inNode, uint32_t outNode)
    {
        NS_LOG_FUNCTION(this);

        for(auto it = m_bidirectionTable.begin(); it != m_bidirectionTable.end(); it++)
        {
            if(it->first == std::pair<uint32_t, uint32_t>(inNode, outNode) && it->second > 0)
            {
                return true;
            }
        }

        return false;

    }

    bool
    DelayForward::HasWaitingData(uint32_t inNode, uint32_t outNode)
    {
        NS_LOG_FUNCTION(this);

        std::map<std::pair<uint32_t, uint32_t>, std::vector<std::string>>::iterator iter;
        
        iter = m_waitingBuffer.find(std::pair<uint32_t, uint32_t>(inNode, outNode));

        if(iter != m_waitingBuffer.end() && !iter->second.empty())
        {
            return true;
        }
        else
        {
            /* code */
            return false;
        }

    }
    

    void
    DelayForward::SendRREQ()
    {
        NS_LOG_FUNCTION(this);

        rapidjson::Document rreqD;

        uint32_t sourceId = GetNode()->GetId();
        int seqNumber = ++m_sequenceNumber;
        //double rreqTimestamp = Simulator::Now().GetSeconds();
        rreqD.SetObject();

        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);
        //rapidjson::Value rreqInfo(rapidjson::kObjectType);

        value = DF_RREQ;
        rreqD.AddMember("message", value, rreqD.GetAllocator());

        value = sourceId;
        rreqD.AddMember("sourceId", value, rreqD.GetAllocator());

        value = m_destinationNodeId;
        rreqD.AddMember("destinationId", value, rreqD.GetAllocator());

        value = sourceId;
        rreqD.AddMember("senderId", value, rreqD.GetAllocator());

        value = seqNumber;
        rreqD.AddMember("sequenceNumber", value, rreqD.GetAllocator());

        value = 0;
        rreqD.AddMember("bidirection", value, rreqD.GetAllocator());

        value = sourceId;
        array.PushBack(value, rreqD.GetAllocator());
        rreqD.AddMember("routes", array, rreqD.GetAllocator());

        rapidjson::StringBuffer rreqBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> rreqWriter(rreqBuffer);
        
        rreqD.Accept(rreqWriter);

        for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            const uint8_t delimiter[] = "#";
            std::cout<<Simulator::Now().GetSeconds()<<" Node " << GetNode()->GetId() << "send RREQ to "<<*i<<std::endl;

            m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(rreqBuffer.GetString()), rreqBuffer.GetSize(), 0);
            m_peersSockets[*i]->Send(delimiter, 1, 0);

            
            //std::cout<<"[Send RREQ] A source node " << GetNode()->GetId() << " send RREQ to " 
            //        <<  *i  << "\n";
            
        }
        

    }

    void
    DelayForward::ForwardRREQ(const Address &from, rapidjson::Document &d)
    {
        NS_LOG_FUNCTION(this);
        /*
        std::cout<<"Node" << GetNode()->GetId() << " forward RREQ which was received by " 
                    <<  InetSocketAddress::ConvertFrom(from).GetIpv4() << "\n";
        */
        unsigned int j = 0;

        std::vector<uint32_t>    tempRoutes;
        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);

        
        m_sequenceTable[d["sourceId"].GetInt()] = d["sequenceNumber"].GetInt();

        for(j = 0; j <d["routes"].Size(); j++)
        {
            tempRoutes.push_back(d["routes"][j].GetInt());
        }

        tempRoutes.push_back((uint32_t)GetNode()->GetId());

        d.RemoveMember("routes");
        d.RemoveMember("senderId");

        value = GetNode()->GetId();
        d.AddMember("senderId", value, d.GetAllocator());

        for(std::vector<uint32_t>::iterator it = tempRoutes.begin(); it != tempRoutes.end();++it)
        {
            value = *it;
            array.PushBack(value, d.GetAllocator());
        }

        d.AddMember("routes", array, d.GetAllocator());

    
        rapidjson::StringBuffer rreqBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> rreqWriter(rreqBuffer);
        
        d.Accept(rreqWriter);

        for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            if(*i != InetSocketAddress::ConvertFrom(from).GetIpv4())
            {
                

                const uint8_t delimiter[] = "#";

                m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(rreqBuffer.GetString()), rreqBuffer.GetSize(), 0);
                m_peersSockets[*i]->Send(delimiter, 1, 0);

                /*
                std::cout<<"[Forward RREQ] A relay node " << GetNode()->GetId() << " send RREQ to " 
                    <<  *i  << "\n";
                */ 
                
            }
            
        }

        
    }

     void
    DelayForward::DelayForwardRREQ(const Address &from, std::string packet)
    {
        NS_LOG_FUNCTION(this);
        
        unsigned int j = 0;
        int                      bidi;
        uint32_t                 inNode;
        uint32_t                 outNode;
        uint32_t                 source;
        uint32_t                 dest;
        std::vector<uint32_t>    tempRoutes;

        
        rapidjson::Document d;
        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);

        d.Parse(packet.c_str());

        inNode = d["senderId"].GetInt();
        source = d["sourceId"].GetInt();
        dest = d["destinationId"].GetInt();

        m_sequenceTable[d["sourceId"].GetInt()] = d["sequenceNumber"].GetInt();

        for(j = 0; j <d["routes"].Size(); j++)
        {
            tempRoutes.push_back(d["routes"][j].GetInt());
        }
        
        tempRoutes.push_back((uint32_t)GetNode()->GetId());

        d.RemoveMember("routes");
        d.RemoveMember("senderId");
        //d.RemoveMember("bidirection");

        value = GetNode()->GetId();
        d.AddMember("senderId", value, d.GetAllocator());
        
        for(std::vector<uint32_t>::iterator it = tempRoutes.begin(); it != tempRoutes.end();++it)
        {
            value = *it;
            array.PushBack(value, d.GetAllocator());
        }

        d.AddMember("routes", array, d.GetAllocator());

        /*
        rapidjson::StringBuffer rreqBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> rreqWriter(rreqBuffer);
        */


        for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            if(*i != InetSocketAddress::ConvertFrom(from).GetIpv4())
            {  

                rapidjson::StringBuffer rreqBuffer;
                rapidjson::Writer<rapidjson::StringBuffer> rreqWriter(rreqBuffer);
                
                d.RemoveMember("bidirection");

                outNode = GetNodeIdFromIP(*i);
                bidi = GetBidirectional(inNode, outNode);
                value = bidi;
                d.AddMember("bidirection", value, d.GetAllocator());

                d.Accept(rreqWriter);
                
                const uint8_t delimiter[] = "#";
                m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(rreqBuffer.GetString()), rreqBuffer.GetSize(), 0);
                m_peersSockets[*i]->Send(delimiter, 1, 0);

                
                //std::cout<<"[Delay Forward RREQ] " <<Simulator::Now().GetSeconds()<<" : A relay node " << GetNode()->GetId() << " send RREQ to " 
                 //   <<  *i  << "\n";
               
                
            }
            
        }

        Simulator::Cancel(m_rreqTimeout[std::pair<uint32_t, uint32_t>(source, dest)]);
        m_rreqDelays.erase(std::pair<uint32_t, uint32_t>(source, dest));
        
    }

    void
    DelayForward::SendRREP(const Address& from, rapidjson::Document &d)
    {
        NS_LOG_FUNCTION(this);

        rapidjson::Document rrepD;

        uint32_t sourceId = GetNode()->GetId();
        int seqNumber = ++m_sequenceNumber;
        //double rrepTimestamp = Simulator::Now().GetSeconds();
        rrepD.SetObject();

        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);
        std::vector<uint32_t>    tempRoutes;

        value = DF_RREP;
        rrepD.AddMember("message", value, rrepD.GetAllocator());

        value = sourceId;
        rrepD.AddMember("sourceId", value, rrepD.GetAllocator());

        value = d["sourceId"].GetInt();
        rrepD.AddMember("destinationId", value, rrepD.GetAllocator());

        value = d["senderId"].GetInt();
        rrepD.AddMember("nextId", value, rrepD.GetAllocator());

        value = sourceId;
        rrepD.AddMember("senderId", value, rrepD.GetAllocator());

        value = seqNumber;
        rrepD.AddMember("sequenceNumber", value, rrepD.GetAllocator());

        for(unsigned int j = 0; j < d["routes"].Size() ; j++)
        {   
            value = d["routes"][j].GetInt();
            m_route.push_back(d["routes"][j].GetInt());
            array.PushBack(value, rrepD.GetAllocator());
        }
        rrepD.AddMember("routes", array, rrepD.GetAllocator());

        rapidjson::StringBuffer rrepBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> rrepWriter(rrepBuffer);
        
        rrepD.Accept(rrepWriter);

        const uint8_t delimiter[] = "#";

        m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()]->Send(reinterpret_cast<const uint8_t*>(rrepBuffer.GetString()), rrepBuffer.GetSize(), 0);
        m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()]->Send(delimiter, 1, 0);

        /*
        std::cout<<"[Send RREP] A destination node " << GetNode()->GetId() << " send RREP to " 
                    <<  InetSocketAddress::ConvertFrom(from).GetIpv4()  << "\n";
        */
        
    }

    void
    DelayForward::ForwardRREP(rapidjson::Document &d)
    {
        NS_LOG_FUNCTION(this);

        unsigned int j = 0;
        uint32_t outNode = 0;
        uint32_t inNode = 0;
        uint32_t nextNode = 0;
        std::vector<uint32_t>    tempRoutes;
        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);

        for(j = 0; j < d["routes"].Size(); j++)
        {
            if(GetNode()->GetId() != (uint32_t)d["routes"][j].GetInt())
            {
                tempRoutes.push_back(d["routes"][j].GetInt());
            }
         
        }

        nextNode = tempRoutes.back();
        inNode = nextNode;
        outNode = d["senderId"].GetInt();
        m_flowTable.push_back(std::pair<uint32_t, uint32_t>(inNode, outNode));

        SetBidiractional();
        
        d.RemoveMember("nextId");
        d.RemoveMember("senderId");
        d.RemoveMember("routes");

        value = nextNode;
        d.AddMember("nextId", value, d.GetAllocator());

        value = GetNode()->GetId();
        d.AddMember("senderId", value, d.GetAllocator());

    

        for(std::vector<uint32_t>::iterator it = tempRoutes.begin(); it != tempRoutes.end();++it)
        {
            value = *it;
            array.PushBack(value, d.GetAllocator());
        }

        d.AddMember("routes", array, d.GetAllocator());

        rapidjson::StringBuffer rrepBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> rrepWriter(rrepBuffer);
        
        d.Accept(rrepWriter);

        const uint8_t delimiter[] = "#";

        m_peersSockets[m_peersIdAddresses[nextNode]]->Send(reinterpret_cast<const uint8_t*>(rrepBuffer.GetString()), rrepBuffer.GetSize(), 0);
        m_peersSockets[m_peersIdAddresses[nextNode]]->Send(delimiter, 1, 0);

        /*
        std::cout<<"[Forward RREP] A relay node " << GetNode()->GetId() << " send RREP to " 
                    <<  m_peersIdAddresses[nextNode] << "\n";
        */
        
    }

    void
    DelayForward::SendData()
    {
        NS_LOG_FUNCTION(this);
        //std::cout<<GetNode()->GetId()<<" Node !@@@@@@@@@@!test!!!!!!!!!\n";
        if(Simulator::Now().GetSeconds()>=68){

            rapidjson::Document dataD;
            
            uint32_t sourceId = GetNode()->GetId();
            int seqNumber = ++m_sequenceNumber;   
            double dataTimestamp = Simulator::Now().GetSeconds();
            dataD.SetObject();

            rapidjson::Value value;

            value = DF_DATA;
            dataD.AddMember("message", value, dataD.GetAllocator());

            value = sourceId;
            dataD.AddMember("senderId", value, dataD.GetAllocator());

            value= m_routingIdTable[m_destinationNodeId];
            dataD.AddMember("nextId", value, dataD.GetAllocator());

            value = sourceId;
            dataD.AddMember("sourceId", value, dataD.GetAllocator());

            value = m_destinationNodeId;
            dataD.AddMember("destinationId", value, dataD.GetAllocator());

            value = seqNumber;
            dataD.AddMember("sequenceNumber", value, dataD.GetAllocator());

            value = dataTimestamp;
            dataD.AddMember("timestamp", value, dataD.GetAllocator());

            rapidjson::StringBuffer dataBuffer;
            rapidjson::Writer<rapidjson::StringBuffer> dataWriter(dataBuffer);
            
            dataD.Accept(dataWriter);

            const uint8_t delimiter[] = "#";

        //std::cout<<"destId : " << m_destinationNodeId
            m_peersSockets[m_routingTable[m_destinationNodeId]]->Send(reinterpret_cast<const uint8_t*>(dataBuffer.GetString()), dataBuffer.GetSize(), 0);
            m_peersSockets[m_routingTable[m_destinationNodeId]]->Send(delimiter, 1, 0);
            m_numberOfSending++;
        }
       
        /*
        std::cout<<"[Send Data] A source node " << GetNode()->GetId() << " send data(dest: "<< m_destinationNodeId <<") to next node  " 
                    <<  m_routingIdTable[m_destinationNodeId] << "\n";
        */

        
        m_copiedRate = m_copiedRate - 1;

        if(m_copiedRate != 0)
        {   
            SendData();
        }
        else
        {
            ReadyToSendData();
            /* code */

        }
        

        //ReadyToSendData();

    }

    void
    DelayForward::ForwardData(rapidjson::Document &d)
    {
        NS_LOG_FUNCTION(this);
        
        //std::cout<<"test1\n";
        //unsigned int j = 0;
        rapidjson::Value value;

        uint32_t dest = d["destinationId"].GetInt();

        rapidjson::StringBuffer dataBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> dataWriter(dataBuffer);

        d.RemoveMember("nextId");
        d.RemoveMember("senderId");

        value = m_routingIdTable[dest];
        d.AddMember("nextId", value, d.GetAllocator());

        value = GetNode()->GetId();
        d.AddMember("senderId", value, d.GetAllocator());

           
        d.Accept(dataWriter);

        const uint8_t delimiter[] = "#";

        m_peersSockets[m_routingTable[dest]]->Send(reinterpret_cast<const uint8_t*>(dataBuffer.GetString()), dataBuffer.GetSize(), 0);
        m_peersSockets[m_routingTable[dest]]->Send(delimiter, 1, 0);

        
        /*
        std::cout<<"[Send Data] A relay node " << GetNode()->GetId() << " forward data(dest: "<< dest <<") to " 
                    << m_routingTable[dest] << "\n";
        */

        /*
        if(m_forwardEvent.IsRunning())
        {
            std::cout << "Node " << GetNode()->GetId() << "canceled m_forwardEvent\n";
            Simulator::Cancel(m_forwardEvent); 
        }
        */
        
        //std::cout<<"Node " << GetNode()->GetId() << " queue size : " << m_waitingForwardBuffer.size() <<"\n";

        if(!m_waitingForwardBuffer.empty() && !m_forwardEvent.IsRunning())
        {
            //std::cout<<"Node " << GetNode()->GetId() << " : m_forwardEvent set up\n";
            m_forwardEvent = Simulator::Schedule(Seconds(m_queueTime), &DelayForward::ForwardTimeoutExpired, this);
        }

    }

    void
    DelayForward::CodingData(std::string packet1, std::string packet2)
    {
        NS_LOG_FUNCTION(this);
        
        
        rapidjson::Document d;
        rapidjson::Document d1;
        rapidjson::Document d2;
        uint32_t dest1;
        uint32_t dest2;

        rapidjson::Value packetInfo1(rapidjson::kObjectType);
        rapidjson::Value packetInfo2(rapidjson::kObjectType);
        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);

        d1.Parse(packet1.c_str());
        d2.Parse(packet2.c_str());

        d.SetObject();

        //std::cout<<"Node " << GetNode()->GetId() << " set up to forward packets using networkcoding\n";

        dest1 = d1["destinationId"].GetInt();
        dest2 = d2["destinationId"].GetInt();

        value = DF_CODING;
        d.AddMember("message", value, d.GetAllocator());

        value = GetNode()->GetId();
        d.AddMember("senderId", value, d.GetAllocator());

        value= m_routingIdTable[dest1];
        d.AddMember("nextId", value, d.GetAllocator());

        /*
        std:: cout<< "!!!!!packet1 src : " << d1["sourceId"].GetInt() <<"\n";
        std:: cout<< "!!!!!packet1 dest : " << d1["destinationId"].GetInt() <<"\n";
        std:: cout<< "!!!!!packet2 src : " << d2["sourceId"].GetInt() <<"\n";
        std:: cout<< "!!!!!packet2 dest : " << d2["destinationId"].GetInt() <<"\n";
        */

        /*
            First packet insert in Coding packet
        */

        //std::cout<<"Node " << GetNode()->GetId() << " : first packet be inserted in networkcoding\n";

        value = DF_DATA;
        packetInfo1.AddMember("message", value, d.GetAllocator());

        value = GetNode()->GetId();
        packetInfo1.AddMember("senderId", value, d.GetAllocator());

        value= m_routingIdTable[dest1];
        packetInfo1.AddMember("nextId", value, d.GetAllocator());
        
        //std::cout<<"test d1 src : " << d1["sourceId"].GetInt() <<"\n";
        value = d1["sourceId"].GetInt();
        packetInfo1.AddMember("sourceId", value, d.GetAllocator());

        value = d1["destinationId"].GetInt();
        packetInfo1.AddMember("destinationId", value, d.GetAllocator());

        value = d1["sequenceNumber"].GetInt();
        packetInfo1.AddMember("sequenceNumber", value, d.GetAllocator());

        value = d1["timestamp"].GetDouble();
        packetInfo1.AddMember("timestamp", value, d.GetAllocator());

        array.PushBack(packetInfo1, d.GetAllocator());

        /*
            Second packet insert in Coding packet
        */
        //std::cout<<"Node " << GetNode()->GetId() << " : second packet be inserted in networkcoding\n";

        value = DF_DATA;
        packetInfo2.AddMember("message", value, d.GetAllocator());

        value = GetNode()->GetId();
        packetInfo2.AddMember("senderId", value, d.GetAllocator());

        value= m_routingIdTable[dest2];
        packetInfo2.AddMember("nextId", value, d.GetAllocator());

        value = d2["sourceId"].GetInt();
        packetInfo2.AddMember("sourceId", value, d.GetAllocator());

        value = d2["destinationId"].GetInt();
        packetInfo2.AddMember("destinationId", value, d.GetAllocator());

        value = d2["sequenceNumber"].GetInt();
        packetInfo2.AddMember("sequenceNumber", value, d.GetAllocator());

        value = d2["timestamp"].GetDouble();
        packetInfo2.AddMember("timestamp", value, d.GetAllocator());

        //std::cout<<"Node " << GetNode()->GetId() << " : all packet inserted in networkcoding\n";
        array.PushBack(packetInfo2, d.GetAllocator());

        d.AddMember("datas", array, d.GetAllocator());

        rapidjson::StringBuffer dataBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> dataWriter(dataBuffer);
        
        d.Accept(dataWriter);

        const uint8_t delimiter[] = "#";

        /*
        m_peersSockets[m_routingTable[dest1]]->Send(reinterpret_cast<const uint8_t*>(dataBuffer.GetString()), dataBuffer.GetSize(), 0);
        m_peersSockets[m_routingTable[dest1]]->Send(delimiter, 1, 0);
        std::cout<<"[Send Coding Data] A relpy node " << GetNode()->GetId() << " forward coding data to next node  " 
                    <<  m_routingIdTable[dest1] << "\n";
        
        m_peersSockets[m_routingTable[dest2]]->Send(reinterpret_cast<const uint8_t*>(dataBuffer.GetString()), dataBuffer.GetSize(), 0);
        m_peersSockets[m_routingTable[dest2]]->Send(delimiter, 1, 0);
        std::cout<<"[Send Coding Data] A relpy node " << GetNode()->GetId() << " forward coding data to next node  " 
                    <<  m_routingIdTable[dest2] << "\n";
        */
        
        m_socket->Send(reinterpret_cast<const uint8_t*>(dataBuffer.GetString()), dataBuffer.GetSize(), 0);
        m_socket->Send(delimiter, 1, 0);
        
        /*
        std::cout<<"[Send Coding Data] A relpy node " << GetNode()->GetId() << " forward(broadcast) coding data to next node  " 
                    <<  m_routingIdTable[dest1] << " and " << m_routingIdTable[dest2]<<"\n";
        */
        
        
        m_numberOfCoding++;
        //m_numberOfCoding=d["bidirection"].GetInt();
    }

    void
    DelayForward::EncodingData(rapidjson::Document &d, int encodingIndex)
    {
        NS_LOG_FUNCTION(this);

        uint32_t dest = d["datas"][encodingIndex]["destinationId"].GetInt();
        uint32_t inNode = d["senderId"].GetInt();
        uint32_t outNode = m_routingIdTable[dest];
        std::string encodingPacket;
        EventId timeout;
        //double  tTime;

        rapidjson::Document p2;
        rapidjson::Value value;
        p2.SetObject();

        
        value = DF_DATA;
        p2.AddMember("message", value, p2.GetAllocator());
        
        value = inNode;
        p2.AddMember("senderId", value, p2.GetAllocator());
        
        value= d["nextId"].GetInt();
        p2.AddMember("nextId", value, p2.GetAllocator());

        value = d["datas"][encodingIndex]["sourceId"].GetInt();;
        p2.AddMember("sourceId", value, p2.GetAllocator());

        value = dest;
        p2.AddMember("destinationId", value, p2.GetAllocator());

        value = d["datas"][encodingIndex]["sequenceNumber"].GetInt();
        p2.AddMember("sequenceNumber", value, p2.GetAllocator());

        value = d["datas"][encodingIndex]["timestamp"].GetDouble();
        p2.AddMember("timestamp", value, p2.GetAllocator());

        
        rapidjson::StringBuffer dataBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> dataWriter(dataBuffer);

        p2.Accept(dataWriter);

        encodingPacket = dataBuffer.GetString();
        
        
        if(HasNetworkCoding(inNode, outNode))
        {
            if(HasWaitingData(outNode,inNode))
            {
                std::map<std::pair<uint32_t, uint32_t>, std::vector<std::string>>::iterator iter;
                std::vector<std::pair<uint32_t, uint32_t>>::iterator q_iter;

                iter = m_waitingBuffer.find(std::pair<uint32_t, uint32_t>(outNode, inNode));
                q_iter = std::find(m_codingQueue.begin(), m_codingQueue.end(), std::pair<uint32_t, uint32_t>(outNode, inNode));

                //std::cout<<iter->second.begin()<<"\n";
                CodingData(*(iter->second.begin()), encodingPacket);

                //m_waitingBuffer.erase(std::pair<uint32_t, uint32_t>(outNode, inNode));
                iter->second.erase(iter->second.begin());
                if(iter->second.empty())
                {
                    m_waitingBuffer.erase(std::pair<uint32_t, uint32_t>(outNode, inNode));
                }
                
                m_codingQueue.erase(q_iter);

                Simulator::Cancel(m_codigTimeout[std::pair<uint32_t, uint32_t>(outNode, inNode)]);
                m_codigTimeout.erase(std::pair<uint32_t, uint32_t>(outNode, inNode));

                //printQueues();

            }
            else
            {
                //tTime =(rand()%2+1)/100;
                m_waitingBuffer[std::pair<uint32_t, uint32_t>(inNode, outNode)].push_back(encodingPacket);
                m_codingQueue.push_back(std::pair<uint32_t, uint32_t>(inNode, outNode));
                //printQueues();

                timeout = Simulator::Schedule(Seconds(m_queueTime), &DelayForward::CodingTimeoutExpired, this);
                m_codigTimeout[std::pair<uint32_t, uint32_t>(inNode, outNode)] = timeout;
            }    
        }
        else
        {   
            //ForwardData(p2);
            //std::cout<<"Func : EncodingData()->ForwardData\n";

            m_waitingForwardBuffer.push(encodingPacket);

            if(!m_forwardEvent.IsRunning())
            {
                m_forwardEvent = Simulator::Schedule(Seconds(m_queueTime), &DelayForward::ForwardTimeoutExpired, this);
            }
        }


    }

    /*
    void
    DelayForward::CodingTimeoutExpired(std::string packet)
    {
        NS_LOG_FUNCTION(this);

        //std::cout<<"Func : CodingTimeoutExpired()\n";

        rapidjson::Document d;

        d.Parse(packet.c_str());

        uint32_t dest = d["destinationId"].GetInt();
        uint32_t inNode = d["senderId"].GetInt();
        uint32_t outNode = m_routingIdTable[dest];

        rapidjson::StringBuffer dataBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> dataWriter(dataBuffer);

        d.Accept(dataWriter);

        //std::cout<<"Node " << GetNode()->GetId() << " : Timeout for waiting packet expired \n";

        ForwardData(d);

        Simulator::Cancel(m_codigTimeout[std::pair<uint32_t, uint32_t>(inNode, outNode)]);
        m_codigTimeout.erase(std::pair<uint32_t, uint32_t>(inNode, outNode));
        m_waitingBuffer.erase(std::pair<uint32_t, uint32_t>(inNode, outNode));
        

    }
    */

     void
    DelayForward::CodingTimeoutExpired(void)
    {
        NS_LOG_FUNCTION(this);

        //std::cout<<"Func : CodingTimeoutExpired()\n";

         if(m_codingQueue.empty())
        {
            //std::cout<<"Queue empty\n";
        }
        else
        {
            rapidjson::Document d;
            std::map<std::pair<uint32_t, uint32_t>, std::vector<std::string>>::iterator iter;
            std::vector<std::pair<uint32_t, uint32_t>>::iterator q_iter =m_codingQueue.begin();

            iter = m_waitingBuffer.find(std::pair<uint32_t, uint32_t>(q_iter->first, q_iter->second));

            
            
            std::string packet = m_waitingBuffer[std::pair<uint32_t, uint32_t>(q_iter->first, q_iter->second)][0];
            
            //std::cout<< packet <<"\n";
            d.Parse(packet.c_str());

            uint32_t dest = d["destinationId"].GetInt();
            uint32_t inNode = d["senderId"].GetInt();
            uint32_t outNode = m_routingIdTable[dest];

            rapidjson::StringBuffer dataBuffer;
            rapidjson::Writer<rapidjson::StringBuffer> dataWriter(dataBuffer);

            d.Accept(dataWriter);

            //std::cout<<"Node " << GetNode()->GetId() << " : Timeout for waiting packet expired \n";

            ForwardData(d);

            Simulator::Cancel(m_codigTimeout[std::pair<uint32_t, uint32_t>(inNode, outNode)]);

            m_codingQueue.erase(q_iter);
            m_codigTimeout.erase(std::pair<uint32_t, uint32_t>(inNode, outNode));

            iter->second.erase(iter->second.begin());
            if(iter->second.empty())
            {
                m_waitingBuffer.erase(std::pair<uint32_t, uint32_t>(outNode, inNode));
            }

            //printQueues();
            //m_waitingBuffer.erase(std::pair<uint32_t, uint32_t>(inNode, outNode));
        }


    }

    void
    DelayForward::ForwardTimeoutExpired()
    {
        NS_LOG_FUNCTION(this);
        
        //std::cout<<"Func : ForwardTimeoutExpired()\n";
        //std::cout<< m_forwardEvent.IsRunning() << "\n";

        if(m_waitingForwardBuffer.empty())
        {
            std::cout<<"Queue empty\n";
        }
        else
        {
            rapidjson::Document d;
            std::string packet = m_waitingForwardBuffer.front();
            m_waitingForwardBuffer.pop();
            //std::cout<<packet<<"\n";
            
            d.Parse(packet.c_str());

            rapidjson::StringBuffer dataBuffer;
            rapidjson::Writer<rapidjson::StringBuffer> dataWriter(dataBuffer);

            d.Accept(dataWriter);

            //std::cout<<"Node " << GetNode()->GetId() << " :Queue expired\n";

            //Simulator::Cancel(m_forwardEvent);

            ForwardData(d);
        }
        
    }
    

    void
    DelayForward::HandleRead(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this);
        Ptr<Packet> packet;
        Address     from;
        
        while((packet = socket->RecvFrom(from)))
        {
            if(packet->GetSize() == 0)
            {
                break;
            }

            if(InetSocketAddress::IsMatchingType(from))
            {   
                if(GetNode()->GetId()==11){
                 std::cout<<"Node " << GetNode()->GetId()
                    << " at time " << Simulator::Now().GetSeconds()
                    << "s packet received " << packet->GetSize()
                    << " bytes from" << InetSocketAddress::ConvertFrom(from).GetIpv4()<<std::endl;
                }
                                
                std::string delimiter = "#";
                std::string parsedPacket;
                size_t pos = 0;
                char *packetInfo = new char[packet->GetSize() + 1];
                std::ostringstream totalStream;

                packet->CopyData(reinterpret_cast<uint8_t*>(packetInfo), packet->GetSize());
                packetInfo[packet->GetSize()] = '\0';

                totalStream << m_bufferedData[from] << packetInfo;
                std::string totalReceivedData(totalStream.str());

                while((pos = totalReceivedData.find(delimiter)) != std::string::npos)
                {
                    parsedPacket = totalReceivedData.substr(0, pos);

                    rapidjson::Document d;
                    d.Parse(parsedPacket.c_str());
                    double nowTimestamp;

                    if(!d.IsObject())
                    {
                        totalReceivedData.erase(0, pos + delimiter.length());
                        continue;
                    }

                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    d.Accept(writer);

                    NS_LOG_INFO("At time " << Simulator::Now().GetSeconds()
                                << "s a node " << GetNode()->GetId() << " received"
                                << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                << " port " << InetSocketAddress::ConvertFrom(from).GetPort()
                                << " with info = " << buffer.GetString());

                    
                    //std::cout<<"Test node : "<<Simulator::Now().GetSeconds() <<" "<< GetNode()->GetId() << " receive packet (" << d["message"].GetInt() << ") \n";
                    
                    switch (d["message"].GetInt())
                    {
                        case DF_RREQ:
                        {
                            //std::cout<<Simulator::Now().GetSeconds()<<" Node " << GetNode()->GetId() << " receive RREQ to "<<d["senderId"].GetInt()<<std::endl;

                            if((uint32_t)d["sourceId"].GetInt() == GetNode()->GetId())
                            {
                                break;
                            }
                            else if((uint32_t)d["destinationId"].GetInt() == GetNode()->GetId())
                            {
                                /* Send RREP */
                                //std::cout<< Simulator::Now().GetSeconds()<< " : " <<"Node "<< GetNode()->GetId()<<" : " <<d["senderId"].GetInt() <<" Received RREQ successfully \n";

                                if(!HasSequenceNumbner((uint32_t)d["sourceId"].GetInt(), d["sequenceNumber"].GetInt()))
                                {
                                    m_sequenceTable[d["sourceId"].GetInt()] =  d["sequenceNumber"].GetInt();
                                    m_sourceId = d["sourceId"].GetInt();
                                    SendRREP(from, d);
                                }
                                else
                                {
                                    /* code */
                                    //std::cout<<"Node "<< GetNode()->GetId() <<" RREQ packet already was received and remove this RREQ \n";
                          
                                }
                                
                                break;
                            }
                            else if(HasSequenceNumbner((uint32_t)d["sourceId"].GetInt(), d["sequenceNumber"].GetInt()))
                            {
                                // The RREQ already was received

                                //std::cout<<"Node "<< GetNode()->GetId() <<" RREQ packet already was received and remove this RREQ \n";
                          
                                break;
                            }
                            else if(m_IsRelayNode)
                            {
                                //Forward RREQ with delay forward
                                // add Delay

                                if(m_IsDfMode)
                                {   
                                    //std::cout << Simulator::Now().GetSeconds()<< " : " <<"Node " << GetNode()->GetId() << " received RREQ (dest: " 
                                    //<< d["destinationId"].GetInt() <<", seq: "<< d["sequenceNumber"].GetInt() 
                                    //<<") packet from " << InetSocketAddress::ConvertFrom(from).GetIpv4() <<"\n";
                            
                                    int bidir = d["bidirection"].GetInt();
                                    uint32_t source = d["sourceId"].GetInt();
                                    uint32_t dest = d["destinationId"].GetInt();
                                    double nowTime = Simulator::Now().GetSeconds();
                                    double delayTime = GetDelayTime(bidir);
                                    double predictionTime = nowTime + delayTime;
                                    EventId rqTime;
                                    srand((unsigned int)time(NULL));

                                    //Time tNext(Seconds(1));
                                    //uint32_t    nodeId = GetNode()->GetId();
                                    //Time tNext(Seconds(1));
                                    //double tenSec = 1000000.0;
                                    //double tr = (rand()%100)/tenSec;
                                        
                                    if(m_rreqTimeout.find(std::pair<uint32_t, uint32_t>(source, dest)) != m_rreqTimeout.end())
                                    {

                                        if(predictionTime < m_rreqDelays[std::pair<uint32_t, uint32_t>(source, dest)])
                                        {
                                                
                                            rqTime = Simulator::Schedule(Seconds(delayTime), &DelayForward::DelayForwardRREQ, this, from, parsedPacket);
                                            m_rreqTimeout[std::pair<uint32_t, uint32_t>(source, dest)] = rqTime;
                                            m_rreqDelays[std::pair<uint32_t, uint32_t>(source, dest)] = predictionTime;

                                            //std::cout<<"Node " << GetNode()->GetId()<< " : Update new RREQ (dealy time : "<<delayTime<<")\n";
                                        }
                                        else
                                        {
                                                //std::cout<<"Node " << GetNode()->GetId()<< " : privious RREQ is faster than new RREQ \n";
                                                    
                                        }
                                            
                                    }
                                    else
                                    {
                                
                                        rqTime = Simulator::Schedule(Seconds(delayTime), &DelayForward::DelayForwardRREQ, this, from, parsedPacket);
                                        m_rreqTimeout[std::pair<uint32_t, uint32_t>(source, dest)] = rqTime;
                                        m_rreqDelays[std::pair<uint32_t, uint32_t>(source, dest)] = predictionTime;
                                        //std::cout<<"Node " << GetNode()->GetId()<< " : Start to wait RREQs (delay time : " << delayTime <<")\n";

                                    } 
                                    
                                    
                                }
                                else
                                {
                                    ForwardRREQ(from, d);
                                }
                                

                                
                                break;
                            }
                            else{
                                break;
                            }
                        }
                        
                        case DF_RREP:
                        {
                            /* code */
                            /*
                            std::cout << "Node " << GetNode()->GetId() << " received RREP (dest: " 
                                    << d["destinationId"].GetInt() <<", seq: "<< d["sequenceNumber"].GetInt() 
                                    <<") packet from " << InetSocketAddress::ConvertFrom(from).GetIpv4() <<"\n";
                            */

                            if(d["sourceId"].GetInt() == (int)GetNode()->GetId())
                            {
                                break;
                            }
                            else if(d["destinationId"].GetInt() == (int)GetNode()->GetId())
                            {
                                /* Send a data */
                                m_routingTable[d["sourceId"].GetInt()] = InetSocketAddress::ConvertFrom(from).GetIpv4();
                                m_routingIdTable[d["sourceId"].GetInt()] = d["senderId"].GetInt();
                                //std::cout<<Simulator::Now().GetSeconds() <<" : Node "<< GetNode()->GetId() <<" ready to send data\n";
                                SendData();
                                break;
                            }
                            else if(d["nextId"].GetInt() == (int)GetNode()->GetId())
                            {
                                //Forward RREQ with delay forward
                                // add Delay
                                m_routingTable[d["sourceId"].GetInt()] = InetSocketAddress::ConvertFrom(from).GetIpv4();
                                m_routingIdTable[d["sourceId"].GetInt()] = d["senderId"].GetInt();
                                ForwardRREP(d);
                                break;
                            }
                            break;
                        }

                        case DF_DATA:
                        {
                            /* code */
                            /*
                            std::cout << "Node " << GetNode()->GetId() << " received DATA (dest: " 
                                    << d["destinationId"].GetInt() 
                                    <<") packet from " << InetSocketAddress::ConvertFrom(from).GetIpv4() <<"\n";
                            */
                            
                            if((uint32_t)d["sourceId"].GetInt() == GetNode()->GetId())
                            {
                                break;
                            }
                            else if((uint32_t)d["destinationId"].GetInt() == GetNode()->GetId())
                            {
                                /* Destination receive the data */
                                //Evaluate performance

                                //std::cout<<"##Node "<< GetNode()->GetId() <<" Successfully received the data.\n";
                                nowTimestamp = (Simulator::Now().GetSeconds()) - d["timestamp"].GetDouble();
                                m_totalReceivedByte += 100;
                                m_numberOfReceiving++;
                                m_latencyTable.push_back(nowTimestamp);

                                uint32_t src = d["sourceId"].GetInt();

                                if(m_receivedPacketTable.find(src) != m_receivedPacketTable.end())
                                {
                                    m_receivedPacketTable[src] += 100;
                                }
                                else
                                {
                                    m_receivedPacketTable[src] = 100;
                                }
                                
                                break;
                            }
                            else if(d["nextId"].GetInt() == (int)GetNode()->GetId())
                            {
                                //Forward RREQ with delay forward
                                // add Delay
                                //std::cout<<"##Node "<< GetNode()->GetId() <<" received the data which will be forwarded.\n";
                               
                                m_totalReceivedByte +=100;

                                
                                uint32_t src = d["sourceId"].GetInt();
                                
                                
                                if(m_receivedPacketTable.find(src) != m_receivedPacketTable.end())
                                {
                                    m_receivedPacketTable[src] += 100;
                                }
                                else
                                {
                                    m_receivedPacketTable[src] = 100;
                                }
                                
                
                                if(m_IsDfMode)
                                {
                                    uint32_t dest = d["destinationId"].GetInt();
                                    uint32_t inNode = d["senderId"].GetInt();
                                    uint32_t outNode = m_routingIdTable[dest];

                                    //double tTime;
                                    EventId timeout;
                                    
                                    if(HasNetworkCoding(inNode, outNode))
                                    {
                                        //std::cout<<"Node " << GetNode()->GetId() << " : received pacekt can be networkcoding\n";
                                        if(HasWaitingData(outNode,inNode))
                                        {
                                            //printQueues();
                                            
                                            //std::cout<<"Node " << GetNode()->GetId() << " : Some packet have been waiting for networkcoding\n";
                                            std::map<std::pair<uint32_t, uint32_t>, std::vector<std::string>>::iterator iter;
                                            std::vector<std::pair<uint32_t, uint32_t>>::iterator q_iter;

                                            iter = m_waitingBuffer.find(std::pair<uint32_t, uint32_t>(outNode, inNode));
                                            q_iter = std::find(m_codingQueue.begin(), m_codingQueue.end(), std::pair<uint32_t, uint32_t>(outNode, inNode));

                                            CodingData(*(iter->second.begin()), parsedPacket);

                                            //m_waitingBuffer.erase(std::pair<uint32_t, uint32_t>(outNode, inNode));
                                            iter->second.erase(iter->second.begin());
                                            if(iter->second.empty())
                                            {
                                                m_waitingBuffer.erase(std::pair<uint32_t, uint32_t>(outNode, inNode));
                                            }

                                            m_codingQueue.erase(q_iter);

                                            Simulator::Cancel(m_codigTimeout[std::pair<uint32_t, uint32_t>(outNode, inNode)]);
                                            m_codigTimeout.erase(std::pair<uint32_t, uint32_t>(outNode, inNode));

                                            //printQueues();

                                        }
                                        else
                                        {
                                            //std::cout<<"Node " << GetNode()->GetId() << " received packet have to wait until other packet\n";
                                            m_waitingBuffer[std::pair<uint32_t, uint32_t>(inNode, outNode)].push_back(parsedPacket);
                                            m_codingQueue.push_back(std::pair<uint32_t, uint32_t>(inNode, outNode));
                                            //printQueues();
                                            
                                            //tTime =(rand()%2+1)/100;
                                            timeout = Simulator::Schedule(Seconds(m_queueTime), &DelayForward::CodingTimeoutExpired, this);
                                            m_codigTimeout[std::pair<uint32_t, uint32_t>(inNode, outNode)] = timeout;
                                            
                                        }
                                        
                                    }
                                    else
                                    {
                                        //ForwardData(d);
                                        //std::cout<<"Node " << GetNode()->GetId() << " insert data in queue\n";
                                        m_waitingForwardBuffer.push(parsedPacket);

                                        if(!m_forwardEvent.IsRunning())
                                        {       
                                            m_forwardEvent = Simulator::Schedule(Seconds(m_queueTime), &DelayForward::ForwardTimeoutExpired, this);
                                          
                                        }
                                    }
                            
                                }
                                else
                                {
                                    //std::cout<<"Node " << GetNode()->GetId() << " insert data in queue\n";

                                    m_waitingForwardBuffer.push(parsedPacket);

                                    //std::cout<<"Node " << GetNode()->GetId() << " queue size : " << m_waitingForwardBuffer.size() <<"\n";

                                    if(!m_forwardEvent.IsRunning())
                                    {
                                        //std::cout<<"Node " << GetNode()->GetId() << " : m_forwardEvent set up\n";
                                        //std::cout<< m_forwardEvent.IsRunning() << "\n";
                                        m_forwardEvent = Simulator::Schedule(Seconds(m_queueTime), &DelayForward::ForwardTimeoutExpired, this);
                                        //std::cout<< m_forwardEvent.IsRunning() << "\n";
                                    }

                                    //ForwardData(d);
                                }
                                
                                break;
                            }
                            break;
                        }

                        case DF_CODING:
                        {
                            /* code */

                            if(std::find(m_peersAddresses.begin(), m_peersAddresses.end(), InetSocketAddress::ConvertFrom(from).GetIpv4()) != m_peersAddresses.end())
                            {
                                /*
                                std::cout << "Node " << GetNode()->GetId() << " received DF_CODING packetket from " 
                                    << InetSocketAddress::ConvertFrom(from).GetIpv4() <<"\n";
                                */

                                uint32_t src1 = d["datas"][0]["sourceId"].GetInt();
                                uint32_t src2 = d["datas"][1]["sourceId"].GetInt();
                                
                                uint32_t dest1 = d["datas"][0]["destinationId"].GetInt();
                                uint32_t dest2 = d["datas"][1]["destinationId"].GetInt();

                                
                                //std::cout <<GetNode()->GetId()<<" :DF-Coding packet (src1: "<< src1 << " dest1: "<< dest1 <<" )\n";
                                //std::cout <<GetNode()->GetId()<<" :DF-Coding packet (src2: "<< src2 << " dest2: "<< dest2 <<" )\n"; 
                                
                                
                                if(dest1 == GetNode()->GetId() && dest2 == GetNode()->GetId())
                                {
                                    //std::cout<<"##Node "<< GetNode()->GetId() <<" Successfully received the data.\n";
                                    nowTimestamp = (Simulator::Now().GetSeconds()) - d["datas"][0]["timestamp"].GetDouble();
                                    m_latencyTable.push_back(nowTimestamp);
                                    nowTimestamp = (Simulator::Now().GetSeconds()) - d["datas"][1]["timestamp"].GetDouble();
                                    m_latencyTable.push_back(nowTimestamp);
                                    m_numberOfReceiving = m_numberOfReceiving+2;
                                    m_totalReceivedByte += 100;
                                    //std::cout<<"test1\n";

                                    
                                    if(m_receivedPacketTable.find(src1) != m_receivedPacketTable.end())
                                    {
                                        m_receivedPacketTable[src1] += 100;
                                    }
                                    else
                                    {
                                        m_receivedPacketTable[src1] = 100;
                                    }

                                    if(m_receivedPacketTable.find(src2) != m_receivedPacketTable.end())
                                    {
                                        m_receivedPacketTable[src2] += 100;
                                    }
                                    else
                                    {
                                        m_receivedPacketTable[src2] = 100;
                                    }
                                    
                                    break;
                                
                                }
                                else if(dest1 == GetNode()->GetId() && dest2 != GetNode()->GetId())
                                {
                                    //std::cout<<"##Node "<< GetNode()->GetId() <<" Successfully received the data.\n";
                                    nowTimestamp = (Simulator::Now().GetSeconds()) - d["datas"][0]["timestamp"].GetDouble();
                                    m_totalReceivedByte += 100;
                                    m_numberOfReceiving++;
                                    m_latencyTable.push_back(nowTimestamp);
                                    //std::cout<<GetNode()->GetId()<<" "<<nowTimestamp<<std::endl;

                                    if(m_receivedPacketTable.find(src1) != m_receivedPacketTable.end())
                                    {
                                        m_receivedPacketTable[src1] += 100;
                                    }
                                    else
                                    {
                                        m_receivedPacketTable[src1] = 100;
                                    }

                                    /*
                                        Check packet 2 for fowarding or coding
                                    */

                                    if((uint32_t)d["datas"][1]["nextId"].GetInt() == GetNode()->GetId())
                                    {
                                        EncodingData(d, 1);
                                    }                    
                                    
                                    break;

                                }
                                else if(dest1 != GetNode()->GetId() && dest2 == GetNode()->GetId())
                                {
                                    //std::cout<<"##Node "<< GetNode()->GetId() <<" Successfully received the data.\n";
                                    nowTimestamp = (Simulator::Now().GetSeconds()) - d["datas"][1]["timestamp"].GetDouble();
                                    m_totalReceivedByte += 100;
                                    m_numberOfReceiving++;
                                    m_latencyTable.push_back(nowTimestamp);

                                    if(m_receivedPacketTable.find(src2) != m_receivedPacketTable.end())
                                    {
                                        m_receivedPacketTable[src2] += 100;
                                    }
                                    else
                                    {
                                        m_receivedPacketTable[src2] = 100;
                                    }


                                    /*
                                        Check packet 1 for fowarding or coding

                                    */

                                    if((uint32_t)d["datas"][0]["nextId"].GetInt() == GetNode()->GetId())
                                    {
                                        EncodingData(d, 0);
                                    }
                                    
                                    break;

                                }
                                else if(dest1 != GetNode()->GetId() && dest2 != GetNode()->GetId())
                                {
                                    /* code */
                                    //std::cout<<"Node "<< GetNode()->GetId() << " : prepare for encoding data \n";

                                    if(((uint32_t)d["datas"][0]["nextId"].GetInt() == GetNode()->GetId()) || ((uint32_t)d["datas"][0]["nextId"].GetInt() == GetNode()->GetId()))
                                    {
                                        //m_totalReceivedByte += 100;

                                        

                                        if(m_receivedPacketTable.find(src1) != m_receivedPacketTable.end())
                                        {
                                            m_receivedPacketTable[src1] += 100;
                                        }
                                        else
                                        {
                                            m_receivedPacketTable[src1] = 100;
                                        }

                                        if(m_receivedPacketTable.find(src2) != m_receivedPacketTable.end())
                                        {
                                            m_receivedPacketTable[src2] += 100;
                                        }
                                        else
                                        {
                                            m_receivedPacketTable[src2] = 100;
                                        }

                                        
                                    }
                                    

                                
                                
                                    if((uint32_t)d["datas"][0]["nextId"].GetInt() == GetNode()->GetId())
                                    {
                                        EncodingData(d, 0);
                                    }
                                    if((uint32_t)d["datas"][1]["nextId"].GetInt() == GetNode()->GetId())
                                    {
                                        EncodingData(d, 1);
                                    }

                                    break;
                                    
                                }
                                else
                                {
                                    /* code */
                                }
                            
                            }
                            else
                            {
                                //std::cout<<"Interference\n";
                            }
                            

                            
                            break;
  
                        }

                        default:
                            break;
                    }



                    totalReceivedData.erase(0, pos + delimiter.length());
                }
                
                m_bufferedData[from] = totalReceivedData;
                delete[] packetInfo;

            }
        
            m_rxTrace(packet);
        }

    }



    double
    DelayForward::GetLatency()
    {
        NS_LOG_FUNCTION(this);


        if(m_latencyTable.size())
        {
            for(std::vector<double>::const_iterator i = m_latencyTable.begin(); i != m_latencyTable.end(); ++i)
            {
                m_sumLatency += *i;
            }

            m_averageLantency = m_sumLatency/m_latencyTable.size();
            
            return m_averageLantency;

        }
        else
        {
            std::cout<<"No latency data\n";
            return 0;
        }
        
    }

    double
    DelayForward::GetThroughput()
    {
        NS_LOG_FUNCTION(this);
        return m_totalReceivedByte/m_sumLatency;
    }

    int
    DelayForward::GetReceivedByte()
    {
        NS_LOG_FUNCTION(this);

        return m_totalReceivedByte;
    }

    int
    DelayForward::GetNumberOfCoding()
    {
        NS_LOG_FUNCTION(this);

        return m_numberOfCoding;
    }

    int
    DelayForward::GetNumberOfSending()
    {
        NS_LOG_FUNCTION(this);
        return m_numberOfSending;
    }

    int
    DelayForward::GetNumberOfReceiving()
    {
        NS_LOG_FUNCTION(this);
        return m_numberOfReceiving;
    }

 

    void
    DelayForward::HandleAccept(Ptr<Socket> socket, const Address& from)
    {
        NS_LOG_FUNCTION(this << socket << from);
        socket->SetRecvCallback(MakeCallback(&DelayForward::HandleRead, this));
    }

    void
    DelayForward::HandlePeerClose(Ptr<Socket> socket) 
    {
        NS_LOG_FUNCTION(this << socket);
    }

    void
    DelayForward::HandlePeerError(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);
    }

    void
    DelayForward::printQueues(void)
    {
        std::cout<<"Node " <<GetNode()->GetId() <<" prints queue stats\n";
        for(std::vector<std::pair<uint32_t, uint32_t>>::iterator i= m_codingQueue.begin(); i != m_codingQueue.end(); i++)
        {
            std::cout<<i->first<< ", " << i->second <<"\n";
        }
        std::cout<<"-----\n";
        for(std::map<std::pair<uint32_t, uint32_t>, std::vector<std::string>>::iterator i=m_waitingBuffer.begin(); i != m_waitingBuffer.end(); i++)
        {
    
            for(std::vector<std::string>::iterator j = i->second.begin(); j != i->second.end(); j++)
            {
                std::cout<<"["<<i->first.first<< ", " << i->first.second<< "], ";
            }
            
            std::cout<<"\n";
        }
    }


}