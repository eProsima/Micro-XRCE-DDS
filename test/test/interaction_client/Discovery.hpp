#ifndef IN_TEST_DISCOVERY_HPP
#define IN_TEST_DISCOVERY_HPP

#include "BigHelloWorld.h"
#include "Gateway.hpp"
#include "Client.hpp"

#include <uxr/client/client.h>
#include <ucdr/microcdr.h>

#include <gtest/gtest.h>
#include <iostream>
#include <thread>

inline
bool operator==(TransportLocator const & lhs, TransportLocator const & rhs)
{
    bool result = false;
    if (lhs.format == rhs.format)
    {
        result = true;
        switch (rhs.format)
        {
        case ADDRESS_FORMAT_MEDIUM:
            for (size_t i = 0; i < sizeof(TransportLocatorMedium::address); ++i)
            {
                result &= lhs._.medium_locator.address[i] == rhs._.medium_locator.address[i];
            }
            result &= lhs._.medium_locator.locator_port == rhs._.medium_locator.locator_port;
            break;
        case ADDRESS_FORMAT_LARGE:
            for (size_t i = 0; i < sizeof(TransportLocatorLarge::address); ++i)
            {
                result &= lhs._.large_locator.address[i] == rhs._.large_locator.address[i];
            }
            result &= lhs._.large_locator.locator_port == rhs._.large_locator.locator_port;
            break;
        default:
            break;
        }
    }
    return result;
}

class Discovery
{
public:
    Discovery(Transport transport, const std::vector<TransportLocator>& agent_locators)
        : agent_locators_{agent_locators}
        , transport_(transport)
    {}

    void unicast(const std::vector<TransportLocator>& discovery_locators)
    {
        uxr_discovery_agents(1, 2000, on_agent_found, this, discovery_locators.data(), discovery_locators.size());
        ASSERT_TRUE(agent_locators_.empty());
    }

    void multicast()
    {
        uxr_discovery_agents_default(1, 1000, on_agent_found, this);
        ASSERT_TRUE(agent_locators_.empty());
    }

private:
    static bool on_agent_found(const TransportLocator* locator, void* args)
    {
        static_cast<Discovery*>(args)->on_agent_found_member(locator);
        return false;
    }

    void on_agent_found_member(const TransportLocator* locator)
    {

        auto it = std::find(agent_locators_.begin(), agent_locators_.end(), *locator);
        if (it != agent_locators_.end())
        {
            switch (locator->format)
            {
                case ADDRESS_FORMAT_MEDIUM:
                {
                    char ip[16];
                    uint16_t port;
                    uxrIpProtocol ip_protocol;
                    uxr_locator_to_ip(locator, ip, sizeof(ip), &port, &ip_protocol);
                    std::cout
                        << "Agent found on IP: " << ip
                        << " and port: " << port
                        << std::endl;
                    std::cout << "Client connecting to Agent" << std::endl;
                    Client client(0.0f, 1);
                    client.init_transport(transport_, ip, std::to_string(port).data());
                    client.close_transport(transport_);
                    break;
                }
                case ADDRESS_FORMAT_LARGE:
                {
                    char ip[46];
                    uint16_t port;
                    uxrIpProtocol ip_protocol;
                    uxr_locator_to_ip(locator, ip, sizeof(ip), &port, &ip_protocol);
                    std::cout
                        << "Agent found on IP: " << ip
                        << " and port: " << port
                        << std::endl;
                    std::cout << "Client connecting to Agent..." << std::endl;
                    Client client(0.0f, 1);
                    client.init_transport(transport_, ip, std::to_string(port).data());
                    client.close_transport(transport_);
                    break;
                }
                default:
                    break;
            }
            agent_locators_.erase(it);
        }

        //ASSERT_TRUE(found); //in multicast, it is possible to read agents out of the tests that will not be found.
    }

    std::vector<TransportLocator> agent_locators_;
    Transport transport_;
};

#endif //IN_TEST_DISCOVERY_HPP
