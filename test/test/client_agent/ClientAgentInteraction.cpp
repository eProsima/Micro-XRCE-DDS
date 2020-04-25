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

#include <Client.hpp>

#include <thread>

class ClientAgentInteraction : public ::testing::TestWithParam<Transport>
{
public:
    const uint16_t AGENT_PORT = 2018 + uint16_t(this->GetParam());
    const float LOST = 0.1f;

    ClientAgentInteraction()
    : transport_(GetParam())
    , client_(0.0f, 8)
    {
        init_agent(AGENT_PORT);
    }

    ~ClientAgentInteraction()
    {}

    void SetUp() override
    {
        if (transport_ == Transport::UDP_IPV4_TRANSPORT || transport_ == Transport::TCP_IPV4_TRANSPORT)
        {
            ASSERT_NO_FATAL_FAILURE(client_.init_transport(transport_, "127.0.0.1", std::to_string(AGENT_PORT).c_str()));
        }
        else
        {
            ASSERT_NO_FATAL_FAILURE(client_.init_transport(transport_, "::1", std::to_string(AGENT_PORT).c_str()));
        }
    }

    void TearDown() override
    {
        ASSERT_NO_FATAL_FAILURE(client_.close_transport(transport_));
    }

    // TODO (#4334): Add serial tests.
    void init_agent(uint16_t port)
    {
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
                agent_udp4_.reset(new eprosima::uxr::UDPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                agent_udp4_->start();
                agent_udp4_->set_verbose_level(6);
                break;
            case Transport::UDP_IPV6_TRANSPORT:
                agent_udp6_.reset(new eprosima::uxr::UDPv6Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                agent_udp6_->start();
                agent_udp6_->set_verbose_level(6);
                break;
            case Transport::TCP_IPV4_TRANSPORT:
                agent_tcp4_.reset(new eprosima::uxr::TCPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                agent_tcp4_->start();
                agent_tcp4_->set_verbose_level(6);
                break;
            case Transport::TCP_IPV6_TRANSPORT:
                agent_tcp6_.reset(new eprosima::uxr::TCPv6Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                agent_tcp6_->start();
                agent_tcp6_->set_verbose_level(6);
                break; 
        }
    }

protected:
    Transport transport_;
    std::unique_ptr<eprosima::uxr::UDPv4Agent> agent_udp4_;
    std::unique_ptr<eprosima::uxr::UDPv6Agent> agent_udp6_;
    std::unique_ptr<eprosima::uxr::TCPv4Agent> agent_tcp4_;
    std::unique_ptr<eprosima::uxr::TCPv6Agent> agent_tcp6_;
    Client client_;
};

INSTANTIATE_TEST_CASE_P(Transports, ClientAgentInteraction, ::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV6_TRANSPORT));

TEST_P(ClientAgentInteraction, InitCloseSession)
{
    //Default behavior
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationXMLBestEffort)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x01, UXR_STATUS_OK, 0));
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationXMLReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationREFBestEffort)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref(1, 0x01, UXR_STATUS_OK, 0));
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationREFReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref(1, 0x80, UXR_STATUS_OK, 0));
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseXMLXMLReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
}

/* TODO (#3589): Fix XML and REF reference issue to enable this test.
TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseXMLREFReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
}
*/

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseREFREFReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref(1, 0x80, UXR_STATUS_OK, 0));
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationNoReplaceReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceReuseReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
}

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
