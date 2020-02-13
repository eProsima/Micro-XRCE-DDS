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

class Discovery
{
public:
    Discovery(int transport, const std::vector<uint16_t>& agent_ports)
    : ip_("127.0.0.1")
    , agent_ports_(agent_ports)
    , transport_(transport)
    {
    }

    void unicast(const std::vector<uint16_t>& discovery_ports)
    {
        std::vector<uxrAgentAddress> agent_list;
        for(uint16_t it : discovery_ports)
        {
            uxrAgentAddress address;
            strcpy(address.ip, ip_.c_str());
            address.port = it;
            agent_list.emplace_back(address);
        }

        uxr_discovery_agents(1, 15000, on_agent_found, this, agent_list.data(), agent_list.size());
        ASSERT_TRUE(agent_ports_.empty());
    }

    void multicast()
    {
        uxr_discovery_agents_default(1, 1000, on_agent_found, this);
        ASSERT_TRUE(agent_ports_.empty());
    }

private:
    static void on_agent_found(const uxrAgentAddress* address, void* args)
    {
        static_cast<Discovery*>(args)->on_agent_found_member(address);
    }

    void on_agent_found_member(const uxrAgentAddress* address)
    {
        std::cout << "Agent found on port: " << address->port << std::endl;


        std::vector<uint16_t>::iterator it = std::find(agent_ports_.begin(), agent_ports_.end(), address->port);

        bool found = it != agent_ports_.end();
        if(found)
        {
            Client client(0.0f, 1);
            std::cout << "Client connecting to " << address->port << std::endl;
            client.init_transport(transport_, address->ip, address->port);
            client.close_transport(transport_);

            agent_ports_.erase(it);
        }

        //ASSERT_TRUE(found); //in multicast, it is possible to read agents out of the tests that will not be found.
    }

    std::string ip_;
    std::vector<uint16_t> agent_ports_;
    int transport_;
};

#endif //IN_TEST_DISCOVERY_HPP
