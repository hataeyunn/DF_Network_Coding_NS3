#include "delay-forward-helper.h"
#include "../model/delay-forward.h"

#include "ns3/names.h"

namespace ns3{


    NS_LOG_COMPONENT_DEFINE("DelayForwardHelper");
    
    DelayForwardHelper::DelayForwardHelper (bool mode, Address local, bool isDfMode, bool isRelayNode, int numberOfNodes, int relaxedK, double relaxedAlpha, 
                        int rate, double dLimit, std::vector<Ipv4Address> &peers, std::map<uint32_t, Ipv4Address> &peersIds)
    {
        m_factory.SetTypeId("ns3::DelayForward");
        m_factory.Set ("Mode", BooleanValue(mode));
        m_factory.Set ("Local", AddressValue(local));
        
        m_IsDfMode = isDfMode;
        m_IsRelayNode = isRelayNode;
        m_numberOfNodes = numberOfNodes;
        m_relaxedK = relaxedK;
        m_relaxedAlpha = relaxedAlpha;
        m_peersAddresses = peers;
        m_peersIdAddresses = peersIds;
        m_rate = rate;
        m_dLimit = dLimit;
    
    }

    void
    DelayForwardHelper::SetAttribute (std::string name, const AttributeValue &value)
    {
        m_factory.Set(name, value);
    }

    void 
    DelayForwardHelper::SetPeersAddresses(const std::vector<Ipv4Address> &peersAddresses)
    {
        NS_LOG_INFO("peer size:" << peersAddresses.size());
        m_peersAddresses = peersAddresses;
    }
    

    void 
    DelayForwardHelper::SetPeersIdAddresses(const std::map<uint32_t, Ipv4Address> &peersIds)
    {
        NS_LOG_INFO("Set up ID-IPv4");
        m_peersIdAddresses = peersIds;
    }

    void
    DelayForwardHelper::SetDelayForwardMode(bool dfMode)
    {
        NS_LOG_INFO(this);
        m_IsDfMode = dfMode;
    }

    void
    DelayForwardHelper::SetRelayMode(bool relayMode)
    {
        NS_LOG_FUNCTION(this);
        m_IsRelayNode = relayMode;
    }

    void
    DelayForwardHelper::SetNumberOfNodes(int numberOfNodes)
    {
        NS_LOG_FUNCTION(this);
        m_numberOfNodes = numberOfNodes;

    }

    void
    DelayForwardHelper::SetRelaxedK(int relaxedK)
    {
        NS_LOG_FUNCTION(this);
        m_relaxedK = relaxedK;
    }

    void
    DelayForwardHelper::SetRelaxedAlpha(double relaxedAlpha)
    {
        NS_LOG_FUNCTION(this);
        m_relaxedAlpha = relaxedAlpha;
        
    }

    void
    DelayForwardHelper::SetRate(int rate)
    {
        NS_LOG_FUNCTION(this);
        m_rate = rate;
    }

    void
    DelayForwardHelper::SetDLimit(double dLimit)
    {
        NS_LOG_FUNCTION(this);
        m_dLimit = dLimit;
    }

    ApplicationContainer
    DelayForwardHelper::Install (Ptr<Node> node) const
    {
        return ApplicationContainer(InstallPriv(node));
    } 

    ApplicationContainer
    DelayForwardHelper::Install (std::string nodeName) const
    {
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer
    DelayForwardHelper::Install (NodeContainer c) const
    {
        ApplicationContainer apps;
        for(NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
        {
            apps.Add(InstallPriv(*i));
        }
        return apps;
    }

    Ptr<Application>
    DelayForwardHelper::InstallPriv (Ptr<Node> node) const
    {
        Ptr<DelayForward> app = m_factory.Create<DelayForward>();
        app->SetPeersAddresses(m_peersAddresses);
        app->SetPeersIdAddresses(m_peersIdAddresses);
        app->SetDelayForwardMode(m_IsDfMode);
        app->SetRelayMode(m_IsRelayNode);
        app->SetNumberOfNodes(m_numberOfNodes);
        app->SetRelaxedK(m_relaxedK);
        app->SetRelaxedAlpha(m_relaxedAlpha);
        app->SetRate(m_rate);
        app->SetDLimit(m_dLimit);
        
        node->AddApplication(app);
        return app;
    }


}