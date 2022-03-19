
//#ifdef DELAY_FORWARD_H
//#define DELAY_FORWARD_H
#include <algorithm>
#include <utility>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/network-module.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"


#include <vector>
#include <map>
#include <queue>

namespace ns3{

    class Socket;
    class Packet;

    enum MessagesType
    {
        DF_RREQ,       //0
        DF_RREP,       //1
        DF_DATA,       //2
        DF_CODING,      //3
    };

    class DelayForward : public Application
    {
        
        public:

            static TypeId GetTypeId(void);
            DelayForward();
            virtual ~DelayForward();

            virtual void StartApplication(void);
            virtual void StopApplication(void);
            
            std::vector<Ipv4Address> SetPeersAdresses(const std::vector<Ipv4Address> &peers);
            
            void SetPeersAddresses(const std::vector<Ipv4Address> &peers);
            void SetPeersIdAddresses(const std::map<uint32_t, Ipv4Address> &peersIds);
            void SetDelayForwardMode(bool dfMode);
            void SetRelayMode(bool relayMode);
            void SetNumberOfNodes(int numberOfNodes);
            void SetRelaxedK(int relaxedK);
            void SetRelaxedAlpha(double relaxedAlpha);
            void SetRate(int rate);
            void SetDLimit(double dLimit);
            
            void ReadyToSendData(void);
            bool HasFlow(uint32_t destId);
            bool HasSequenceNumbner(uint32_t source, int sequence);
            void SetBidiractional(void);
            int GetBidirectional(uint32_t inNode, uint32_t outNode);
            uint32_t GetNodeIdFromIP(Ipv4Address ip);
            double GetDelayTime(int bidir);

            bool HasNetworkCoding(uint32_t inNode, uint32_t outNode);
            bool HasWaitingData(uint32_t inNode, uint32_t outNode);


            void SendRREQ(void);
            void ForwardRREQ(const Address& from, rapidjson::Document &d);
            void DelayForwardRREQ(const Address& from, std::string packet);
            void SendRREP(const Address& from, rapidjson::Document &d);
            void ForwardRREP(rapidjson::Document &d);
            void SendData(void);
            void ForwardData(rapidjson::Document &d);
            void CodingData(std::string packet1, std::string packet2);
            void EncodingData(rapidjson::Document &d, int encodingIndex);
            void CodingTimeoutExpired(void);
            //void CodingTimeoutExpired(std::string packet);
            void ForwardTimeoutExpired(void);

            double GetLatency(void);
            double GetThroughput(void);
            int GetReceivedByte(void);
            int GetNumberOfCoding(void);
            int GetNumberOfSending(void);
            int GetNumberOfReceiving(void);
            void printQueues(void);

            
            void HandleRead(Ptr<Socket> socket);
            void HandleAccept(Ptr<Socket> socket, const Address& from);
            void HandlePeerClose(Ptr<Socket> socket);
            void HandlePeerError(Ptr<Socket> socket);

        
        private: 
            
            bool            m_mode;
            bool            m_IsDfMode;
            bool            m_IsRelayNode;
            Address         m_local;
            uint32_t        m_destinationId;
            uint32_t        m_sourceId;
            TypeId          m_tid;
            DataRate        m_dataRate;
            double          m_dLimit;
            int             m_numberOfNodes;
            int             m_relaxedK;
            int             m_rate;
            int             m_copiedRate;
            double          m_relaxedAlpha;
            double          m_queueTime;

            double          m_bestDelay;

            int             m_totalReceivedByte;
            double          m_throughput;
            double          m_averageLantency;
            int             m_numberOfCoding;
            int             m_numberOfSending;
            int             m_numberOfReceiving;
            double          m_sumLatency;
            int             m_numberofbidir;
            int             m_sequenceNumber;
            uint32_t        m_destinationNodeId;

            std::vector<uint32_t>                       m_peersIds;
            std::vector<Ipv4Address>                    m_peersAddresses;
            std::map<uint32_t, Ipv4Address>             m_peersIdAddresses;
            //std::vector<Ipv4Address>                m_destinationsAddresses;
            std::map<Ipv4Address, Ptr<Socket>>          m_peersSockets;
            std::map<uint32_t, Ipv4Address>             m_routingTable;
            std::map<uint32_t, uint32_t>                m_routingIdTable;
            std::map<Address, std::string>              m_bufferedData;
            std::map<uint32_t, int>                     m_sequenceTable;
            std::vector<std::pair<uint32_t, uint32_t>>              m_flowTable;
            std::queue<std::string>                                 m_waitingForwardBuffer;
            std::vector<std::pair<uint32_t, uint32_t>>              m_codingQueue;
            std::map<std::pair<uint32_t, uint32_t>, int>            m_bidirectionTable;
            std::map<std::pair<uint32_t, uint32_t>, std::vector<std::string>>    m_waitingBuffer;
            std::map<std::pair<uint32_t, uint32_t>, double>         m_rreqDelays;
            std::map<std::pair<uint32_t, uint32_t>, EventId>        m_codigTimeout;
            std::map<std::pair<uint32_t, uint32_t>, EventId>        m_rreqTimeout;
            std::map<uint32_t, int>                                 m_receivedPacketTable;
            std::vector<double>                                     m_latencyTable;
            std::vector<uint32_t>                                   m_route;

            Ptr<Socket>     m_socket;

            uint32_t        m_numberOfPeers;
            uint32_t        m_packetSize;
            //uint32_t        m_packetSent;
            uint32_t        m_bidirectional;

            EventId         m_dataEvent;
            EventId         m_delayEvent;
            EventId         m_CodingEvent;
            EventId         m_forwardEvent;
        

            TracedCallback<Ptr<const Packet>>   m_txTrace;
            TracedCallback<Ptr<const Packet>>   m_rxTrace;


    };

}

//#endif
