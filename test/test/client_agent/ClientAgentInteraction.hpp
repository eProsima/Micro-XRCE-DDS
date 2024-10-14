#ifndef IN_TEST_CLIENT_INT_HPP
#define IN_TEST_CLIENT_INT_HPP

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

class Agent
{
public:
    Agent(Transport transport,
          MiddlewareKind middleware,
          const uint16_t port)
        : transport_(transport)
        , middleware_{}
        , port_(port)
    {
        switch (middleware)
        {
            case MiddlewareKind::FASTDDS:
                middleware_ = eprosima::uxr::Middleware::Kind::FASTDDS;
                break;
            case MiddlewareKind::CED:
                middleware_ = eprosima::uxr::Middleware::Kind::CED;
                break;
        }
    }

    ~Agent()
    {}

    void start()
    {
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            {
                agent_udp4_.reset(new eprosima::uxr::UDPv4Agent(port_, middleware_));
                agent_udp4_->set_verbose_level(6);
                ASSERT_TRUE(agent_udp4_->start());
                break;
            }
            case Transport::UDP_IPV6_TRANSPORT:
            {
                agent_udp6_.reset(new eprosima::uxr::UDPv6Agent(port_, middleware_));
                agent_udp6_->set_verbose_level(6);
                ASSERT_TRUE(agent_udp6_->start());
                break;
            }
            case Transport::TCP_IPV4_TRANSPORT:
            {
                agent_tcp4_.reset(new eprosima::uxr::TCPv4Agent(port_, middleware_));
                agent_tcp4_->set_verbose_level(6);
                ASSERT_TRUE(agent_tcp4_->start());
                break;
            }
            case Transport::TCP_IPV6_TRANSPORT:
            {
                agent_tcp6_.reset(new eprosima::uxr::TCPv6Agent(port_, middleware_));
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

    void stop()
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

private:
    Transport transport_;
    std::unique_ptr<eprosima::uxr::UDPv4Agent> agent_udp4_;
    std::unique_ptr<eprosima::uxr::UDPv6Agent> agent_udp6_;
    std::unique_ptr<eprosima::uxr::TCPv4Agent> agent_tcp4_;
    std::unique_ptr<eprosima::uxr::TCPv6Agent> agent_tcp6_;
    std::unique_ptr<eprosima::uxr::CustomAgent> agent_custom_;
    eprosima::uxr::CustomEndPoint agent_custom_endpoint_;

    eprosima::uxr::Middleware::Kind middleware_;
    uint16_t port_;
};

class ClientAgentInteraction : public ::testing::TestWithParam<std::tuple<Transport, MiddlewareKind>>
{
public:
    const uint16_t AGENT_PORT = 2018 + uint16_t(std::get<0>(GetParam()));
    const float LOST = 0.1f;
    static const uint8_t INIT_CLOSE_RETRIES = 20;

    ClientAgentInteraction()
        : transport_(std::get<0>(GetParam()))
        , client_(0.0f, 8)
        , agent_(transport_, (MiddlewareKind) std::get<1>(GetParam()), AGENT_PORT)
    {}

    ~ClientAgentInteraction()
    {
    }

    void SetUp() override
    {
        agent_.start();
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
        agent_.stop();
    }


protected:
    Transport transport_;
    Client client_;
    Agent agent_;
};

#endif // ifndef IN_TEST_CLIENT_INT_HPP