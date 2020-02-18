#include <gtest/gtest.h>

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPv4AgentWindows.hpp>
#include <uxr/agent/transport/udp/UDPv6AgentWindows.hpp>
#include <uxr/agent/transport/tcp/TCPv4AgentWindows.hpp>
#include <uxr/agent/transport/tcp/TCPv6AgentWindows.hpp>
#else
#include <uxr/agent/transport/udp/UDPv4AgentLinux.hpp>
// #include <uxr/agent/transport/udp/UDPv6AgentLinux.hpp>
// #include <uxr/agent/transport/tcp/TCPv4AgentLinux.hpp>
// #include <uxr/agent/transport/tcp/TCPv6AgentLinux.hpp>
#endif

#include <Discovery.hpp>

#include <thread>

class DiscoveryIntegration : public ::testing::TestWithParam<Transport>
{
public:
    const uint16_t AGENT_PORT = 2018;
    const uint16_t DISCOVERY_PORT = eprosima::uxr::DISCOVERY_PORT;

    DiscoveryIntegration()
    : transport_(GetParam())
    {
    }

    ~DiscoveryIntegration()
    {
    }

    std::vector<uint16_t> init_scenario(size_t number)
    {
        std::vector<uint16_t> agent_ports;
        std::vector<uint16_t> discovery_ports;
        for(size_t i = 0; i < number; i++)
        {
            uint16_t agent_port = uint16_t(AGENT_PORT + i);
            uint16_t discovery_port = uint16_t(DISCOVERY_PORT + i);
            create_agent(agent_port, discovery_port);
            agent_ports.push_back(agent_port);
            discovery_ports.push_back(discovery_port);
        }

        discovery_.reset(new Discovery(transport_, agent_ports));

        return discovery_ports;
    }

    void create_agent(uint16_t port, uint16_t discovery_port)
    {
        std::unique_ptr<eprosima::uxr::UDPv4Agent> agent;
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
                agent.reset(new eprosima::uxr::UDPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                break;
            case Transport::UDP_IPV6_TRANSPORT:
                // agent.reset(new eprosima::uxr::UDPv6Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                // break;
            case Transport::TCP_IPV4_TRANSPORT:
                // agent.reset(new eprosima::uxr::TCPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                // break;
            case Transport::TCP_IPV6_TRANSPORT:
                // agent.reset(new eprosima::uxr::TCPv6Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                break;
        }
        agent->run();
        agent->enable_discovery(discovery_port);
        agents_.push_back(std::move(agent));
    }

protected:
    Transport transport_;
    std::unique_ptr<Discovery> discovery_;

private:
    std::vector<std::unique_ptr<eprosima::uxr::UDPv4Agent>> agents_;
};

INSTANTIATE_TEST_CASE_P(Transports, DiscoveryIntegration, ::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::TCP_IPV6_TRANSPORT), ::testing::PrintToStringParamName());

TEST_P(DiscoveryIntegration, DiscoveryUnicast)
{
    std::vector<uint16_t> discovery_ports = init_scenario(4);
    discovery_->unicast(discovery_ports);
}

TEST_P(DiscoveryIntegration, DiscoveryMulticast)
{
    init_scenario(1);
    discovery_->multicast();
}

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
