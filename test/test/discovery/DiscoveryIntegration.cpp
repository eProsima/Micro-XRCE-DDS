#include <gtest/gtest.h>

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPv4AgentWindows.hpp>
#include <uxr/agent/transport/udp/UDPv6AgentWindows.hpp>
#include <uxr/agent/transport/tcp/TCPv4AgentWindows.hpp>
#include <uxr/agent/transport/tcp/TCPv6AgentWindows.hpp>
#else
#include <uxr/agent/transport/udp/UDPv4AgentLinux.hpp>
#include <uxr/agent/transport/udp/UDPv6AgentLinux.hpp>
#include <uxr/agent/transport/tcp/TCPv4AgentLinux.hpp>
#include <uxr/agent/transport/tcp/TCPv6AgentLinux.hpp>
#endif

#include <Discovery.hpp>

#include <thread>

#define LOCAL_HOST_IPV4 "127.0.0.1"
#define LOCAL_HOST_IPV6 "::1"

class DiscoveryIntegration : public ::testing::TestWithParam<std::tuple<Transport, MiddlewareKind>>
{
public:
    const uint16_t AGENT_PORT = 2018;
    const uint16_t DISCOVERY_PORT = eprosima::uxr::DISCOVERY_PORT;

    DiscoveryIntegration()
        : transport_(std::get<0>(GetParam()))
        , middleware_{}
    {
        switch (std::get<1>(GetParam()))
        {
        case MiddlewareKind::FASTDDS:
            middleware_ = eprosima::uxr::Middleware::Kind::FASTDDS;
            break;
        case MiddlewareKind::FASTRTPS:
            middleware_ = eprosima::uxr::Middleware::Kind::FASTRTPS;
            break;
        case MiddlewareKind::CED:
            middleware_ = eprosima::uxr::Middleware::Kind::CED;
            break;
        }
    }

    ~DiscoveryIntegration() = default;

    std::vector<TransportLocator> init_scenario(size_t number)
    {
        std::vector<TransportLocator> agent_locators{};
        std::vector<TransportLocator> discovery_locators{};

        if (Transport::UDP_IPV4_TRANSPORT == transport_ || Transport::TCP_IPV4_TRANSPORT == transport_)
        {
            TransportLocator current_agent_locator;
            uxr_ip_to_locator(LOCAL_HOST_IPV4, DISCOVERY_PORT, UXR_IPv4, &current_agent_locator);
            TransportLocator current_discovery_locator;
            uxr_ip_to_locator(LOCAL_HOST_IPV4, DISCOVERY_PORT, UXR_IPv4, &current_discovery_locator);

            for (size_t i = 0; i < number; i++)
            {
                uint16_t agent_port = uint16_t(AGENT_PORT + i);
                uint16_t discovery_port = uint16_t(DISCOVERY_PORT + i);
                current_agent_locator._.medium_locator.locator_port = agent_port;
                agent_locators.push_back(current_agent_locator);
                current_discovery_locator._.medium_locator.locator_port = discovery_port;
                discovery_locators.push_back(current_discovery_locator);
                create_agent(agent_port, discovery_port);
            }
        }
        else
        {
            TransportLocator current_agent_locator;
            uxr_ip_to_locator(LOCAL_HOST_IPV6, DISCOVERY_PORT, UXR_IPv6, &current_agent_locator);
            TransportLocator current_discovery_locator;
            uxr_ip_to_locator(LOCAL_HOST_IPV4, DISCOVERY_PORT, UXR_IPv4, &current_discovery_locator);

            for (size_t i = 0; i < number; i++)
            {
                uint16_t agent_port = uint16_t(AGENT_PORT + i);
                uint16_t discovery_port = uint16_t(DISCOVERY_PORT + i);
                current_agent_locator._.large_locator.locator_port = agent_port;
                agent_locators.push_back(current_agent_locator);
                current_discovery_locator._.medium_locator.locator_port = discovery_port;
                discovery_locators.push_back(current_discovery_locator);
                create_agent(agent_port, discovery_port);
            }
        }

        discovery_.reset(new Discovery(transport_, agent_locators));

        return discovery_locators;
    }

    void create_agent(uint16_t port, uint16_t discovery_port)
    {
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            {
                std::unique_ptr<eprosima::uxr::UDPv4Agent> agent;
                agent.reset(new eprosima::uxr::UDPv4Agent(port, middleware_));
                agent->start();
                agent->set_verbose_level(6);
                agent->enable_discovery(discovery_port);
                agents_udp4_.push_back(std::move(agent));
                break;
            }
            case Transport::UDP_IPV6_TRANSPORT:
            {
                std::unique_ptr<eprosima::uxr::UDPv6Agent> agent;
                agent.reset(new eprosima::uxr::UDPv6Agent(port, middleware_));
                agent->start();
                agent->set_verbose_level(6);
                agent->enable_discovery(discovery_port);
                agents_udp6_.push_back(std::move(agent));
                break;
            }
            case Transport::TCP_IPV4_TRANSPORT:
            {
                std::unique_ptr<eprosima::uxr::TCPv4Agent> agent;
                agent.reset(new eprosima::uxr::TCPv4Agent(port, middleware_));
                agent->start();
                agent->set_verbose_level(6);
                agent->enable_discovery(discovery_port);
                agents_tcp4_.push_back(std::move(agent));
                break;
            }
            case Transport::TCP_IPV6_TRANSPORT:
            {
                std::unique_ptr<eprosima::uxr::TCPv6Agent> agent;
                agent.reset(new eprosima::uxr::TCPv6Agent(port, middleware_));
                agent->start();
                agent->set_verbose_level(6);
                agent->enable_discovery(discovery_port);
                agents_tcp6_.push_back(std::move(agent));
                break;
            }
        }
    }

protected:
    Transport transport_;
    eprosima::uxr::Middleware::Kind middleware_;
    std::unique_ptr<Discovery> discovery_;

private:
    std::vector<std::unique_ptr<eprosima::uxr::UDPv4Agent>> agents_udp4_;
    std::vector<std::unique_ptr<eprosima::uxr::UDPv6Agent>> agents_udp6_;
    std::vector<std::unique_ptr<eprosima::uxr::TCPv4Agent>> agents_tcp4_;
    std::vector<std::unique_ptr<eprosima::uxr::TCPv6Agent>> agents_tcp6_;
};

// TODO(pablogs): Fix this test, randomly fails. Maybe a timeout?
// TEST_P(DiscoveryIntegration, DiscoveryUnicast)
// {
//     std::vector<TransportLocator> discovery_locators = init_scenario(4);
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     discovery_->unicast(discovery_locators);
// }

TEST_P(DiscoveryIntegration, DiscoveryMulticast)
{
    init_scenario(1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    discovery_->multicast();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    Transports,
    DiscoveryIntegration,
    ::testing::Combine(
        ::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::TCP_IPV6_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS, MiddlewareKind::FASTRTPS, MiddlewareKind::CED)));

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
