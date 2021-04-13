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

#include <uxr/agent/transport/custom/CustomAgent.hpp>

#include <Client.hpp>

#include <thread>

class ClientAgentInteraction : public ::testing::TestWithParam<std::tuple<Transport, MiddlewareKind>>
{
public:
    const uint16_t AGENT_PORT = 2018 + uint16_t(std::get<0>(GetParam()));
    const float LOST = 0.1f;
    static const uint8_t INIT_CLOSE_RETRIES = 20;

    ClientAgentInteraction()
        : transport_(std::get<0>(GetParam()))
        , middleware_{}
        , client_(0.0f, 8)
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

    ~ClientAgentInteraction()
    {}

    void SetUp() override
    {
        start_agent(AGENT_PORT);
        switch (transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            case Transport::TCP_IPV4_TRANSPORT:
            {
                ASSERT_NO_FATAL_FAILURE(client_.init_transport(transport_, "127.0.0.1", std::to_string(AGENT_PORT).c_str()));
                break;
            }
            case Transport::UDP_IPV6_TRANSPORT:
            case Transport::TCP_IPV6_TRANSPORT:
            {
                ASSERT_NO_FATAL_FAILURE(client_.init_transport(transport_, "::1", std::to_string(AGENT_PORT).c_str()));
                break;
            }
            case Transport::CUSTOM_WITHOUT_FRAMING:
            case Transport::CUSTOM_WITH_FRAMING:
            {
                ASSERT_NO_FATAL_FAILURE(client_.init_transport(transport_, NULL, NULL));
                break;
            }
        }
    }

    void TearDown() override
    {
        ASSERT_NO_FATAL_FAILURE(client_.close_transport(transport_));
        stop_agent(AGENT_PORT);
    }

    // TODO (#4334 - @jamoralp): Add serial tests.
    void start_agent(uint16_t port)
    {
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            {
                agent_udp4_.reset(new eprosima::uxr::UDPv4Agent(port, middleware_));
                agent_udp4_->set_verbose_level(6);
                ASSERT_TRUE(agent_udp4_->start());
                break;
            }
            case Transport::UDP_IPV6_TRANSPORT:
            {
                agent_udp6_.reset(new eprosima::uxr::UDPv6Agent(port, middleware_));
                agent_udp6_->set_verbose_level(6);
                ASSERT_TRUE(agent_udp6_->start());
                break;
            }
            case Transport::TCP_IPV4_TRANSPORT:
            {
                agent_tcp4_.reset(new eprosima::uxr::TCPv4Agent(port, middleware_));
                agent_tcp4_->set_verbose_level(6);
                ASSERT_TRUE(agent_tcp4_->start());
                break;
            }
            case Transport::TCP_IPV6_TRANSPORT:
            {
                agent_tcp6_.reset(new eprosima::uxr::TCPv6Agent(port, middleware_));
                agent_tcp6_->set_verbose_level(6);
                ASSERT_TRUE(agent_tcp6_->start());
                break;
            }
            case Transport::CUSTOM_WITHOUT_FRAMING:
            {
                try
                {
                    agent_custom_endpoint_.add_member<uint32_t>("index");
                }
                catch(const std::exception& /*e*/)
                {
                    // Do nothing
                }
                

                agent_custom_.reset(new eprosima::uxr::CustomAgent(
                    "custom_agent",
                    &agent_custom_endpoint_,
                    middleware_,
                    false,
                    agent_custom_transport_open,
                    agent_custom_transport_close,
                    agent_custom_transport_write_packet,
                    agent_custom_transport_read_packet));
                agent_custom_->set_verbose_level(6);
                ASSERT_TRUE(agent_custom_->start());
                break;
            }
            case Transport::CUSTOM_WITH_FRAMING:
            {
                try
                {
                    agent_custom_endpoint_.add_member<uint32_t>("index");
                }
                catch(const std::exception& /*e*/)
                {
                    // Do nothing
                }

                agent_custom_.reset(new eprosima::uxr::CustomAgent(
                    "custom_agent",
                    &agent_custom_endpoint_,
                    middleware_,
                    true,
                    agent_custom_transport_open,
                    agent_custom_transport_close,
                    agent_custom_transport_write_stream,
                    agent_custom_transport_read_stream));
                agent_custom_->set_verbose_level(6);
                ASSERT_TRUE(agent_custom_->start());
                break;
            }
            
        }
    }

    void stop_agent(uint16_t port)
    {
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            {
                ASSERT_TRUE(agent_udp4_->stop());
                break;
            }
            case Transport::UDP_IPV6_TRANSPORT:
            {
                ASSERT_TRUE(agent_udp6_->stop());
                break;
            }
            case Transport::TCP_IPV4_TRANSPORT:
            {
                ASSERT_TRUE(agent_tcp4_->stop());
                break;
            }
            case Transport::TCP_IPV6_TRANSPORT:
            {
                ASSERT_TRUE(agent_tcp6_->stop());
                break;
            }
            case Transport::CUSTOM_WITHOUT_FRAMING:
            case Transport::CUSTOM_WITH_FRAMING:
            {
                ASSERT_TRUE(agent_custom_->stop());
                break;            
            }
        }
    }

protected:
    Transport transport_;
    std::unique_ptr<eprosima::uxr::UDPv4Agent> agent_udp4_;
    std::unique_ptr<eprosima::uxr::UDPv6Agent> agent_udp6_;
    std::unique_ptr<eprosima::uxr::TCPv4Agent> agent_tcp4_;
    std::unique_ptr<eprosima::uxr::TCPv6Agent> agent_tcp6_;
    std::unique_ptr<eprosima::uxr::CustomAgent> agent_custom_;
    eprosima::uxr::CustomEndPoint agent_custom_endpoint_;

    eprosima::uxr::Middleware::Kind middleware_;
    Client client_;
};

TEST_P(ClientAgentInteraction, InitCloseSession)
{
    for (int i = 0; i < ClientAgentInteraction::INIT_CLOSE_RETRIES; ++i)
    {
        TearDown();
        SetUp();
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationXMLBestEffort)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationXMLReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationBINBestEffort)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            // Not implemented
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationBINReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            // Not implemented
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationREFBestEffort)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationREFReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseXMLXMLReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
    }
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
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationNoReplaceReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceReuseReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, PingFromClientToAgent)
{
    const Transport transport_kind(std::get<0>(GetParam()));
    ASSERT_NO_FATAL_FAILURE(client_.ping_agent(transport_kind));
}

INSTANTIATE_TEST_CASE_P(
    Transports,
    ClientAgentInteraction,
    ::testing::Combine(
        ::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV6_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS, MiddlewareKind::FASTRTPS, MiddlewareKind::CED)));

INSTANTIATE_TEST_CASE_P(
    CustomTransports,
    ClientAgentInteraction,
    ::testing::Combine(
        ::testing::Values(Transport::CUSTOM_WITHOUT_FRAMING, Transport::CUSTOM_WITH_FRAMING),
        ::testing::Values(MiddlewareKind::FASTDDS)));

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
