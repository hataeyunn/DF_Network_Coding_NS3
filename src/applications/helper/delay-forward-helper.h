#ifndef DELAY_FORWARD_HELPER_H
#define DELAY_FORWARD_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/address.h"
#include <vector>
#include <map>

namespace ns3{

    class DelayForwardHelper{

        public:
            DelayForwardHelper(bool mode, Address local, bool isDfMode, bool isRelayNode, int numberOfNodes, int relaxedK, double relaxedAlpha,
                                int rate, double dLimit, std::vector<Ipv4Address> &peers, std::map<uint32_t, Ipv4Address> &peersIds);
            void SetAttribute(std::string name, const AttributeValue &value);
            void SetPeersAddresses(const std::vector<Ipv4Address> &peers);
            void SetPeersIdAddresses(const std::map<uint32_t, Ipv4Address> &peersIds);
            void SetDelayForwardMode(bool dfMode);
            void SetRelayMode(bool relayMode);
            void SetNumberOfNodes(int numberOfNodes);
            void SetRelaxedK(int relaxedK);
            void SetRelaxedAlpha(double relaxedAlpha);
            void SetRate(int rate);
            void SetDLimit(double dLimit);

            ApplicationContainer Install (Ptr<Node> node) const;
            ApplicationContainer Install (std::string nodeName) const;
            ApplicationContainer Install (NodeContainer c) const;

        private:
            Ptr<Application> InstallPriv (Ptr<Node> node) const;
            ObjectFactory                           m_factory;
            std::vector<Ipv4Address>                m_peersAddresses;
            std::map<uint32_t, Ipv4Address>         m_peersIdAddresses;
            bool                                    m_IsDfMode;
            bool                                    m_IsRelayNode;
            int                                     m_numberOfNodes;
            int                                     m_relaxedK;
            int                                     m_rate;
            double                                  m_relaxedAlpha;
            double                                  m_dLimit;


    };


}

#endif