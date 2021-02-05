#include <gtest/gtest.h>

#include <Client.hpp>
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


#include <thread>

class PublisherSubscriberNoLost : public ::testing::TestWithParam<std::tuple<Transport, MiddlewareKind, float>>
{
public:
    const uint16_t AGENT_PORT = 2018 + uint16_t(std::get<0>(this->GetParam()));

    PublisherSubscriberNoLost()
        : transport_(std::get<0>(GetParam()))
        , middleware_{}
        , publisher_(std::get<2>(GetParam()), 8)
        , subscriber_(std::get<2>(GetParam()), 8)
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
        init_agent(AGENT_PORT);
    }

    ~PublisherSubscriberNoLost()
    {}

    void SetUp() override
    {
        if (transport_ == Transport::UDP_IPV4_TRANSPORT || transport_ == Transport::TCP_IPV4_TRANSPORT)
        {
            ASSERT_NO_FATAL_FAILURE(publisher_.init_transport(transport_, "127.0.0.1", std::to_string(AGENT_PORT).c_str()));
            ASSERT_NO_FATAL_FAILURE(subscriber_.init_transport(transport_, "127.0.0.1", std::to_string(AGENT_PORT).c_str()));
        }
        else
        {
            ASSERT_NO_FATAL_FAILURE(publisher_.init_transport(transport_, "::1", std::to_string(AGENT_PORT).c_str()));
            ASSERT_NO_FATAL_FAILURE(subscriber_.init_transport(transport_, "::1", std::to_string(AGENT_PORT).c_str()));
        }

        switch (std::get<1>(GetParam()))
        {
        case MiddlewareKind::FASTDDS:
            ASSERT_NO_FATAL_FAILURE(publisher_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(subscriber_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        case MiddlewareKind::FASTRTPS:
            ASSERT_NO_FATAL_FAILURE(publisher_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(subscriber_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        case MiddlewareKind::CED:
            ASSERT_NO_FATAL_FAILURE(publisher_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(subscriber_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
    }

    void TearDown() override
    {
        ASSERT_NO_FATAL_FAILURE(publisher_.close_transport(transport_));
        ASSERT_NO_FATAL_FAILURE(subscriber_.close_transport(transport_));
    }

    void init_agent(uint16_t port)
    {
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
                agent_udp4_.reset(new eprosima::uxr::UDPv4Agent(port, middleware_));
                agent_udp4_->set_verbose_level(6);
                ASSERT_TRUE(agent_udp4_->start());
                break;
            case Transport::UDP_IPV6_TRANSPORT:
                agent_udp6_.reset(new eprosima::uxr::UDPv6Agent(port, middleware_));
                agent_udp6_->set_verbose_level(6);
                ASSERT_TRUE(agent_udp6_->start());
                break;
            case Transport::TCP_IPV4_TRANSPORT:
                agent_tcp4_.reset(new eprosima::uxr::TCPv4Agent(port, middleware_));
                agent_tcp4_->set_verbose_level(6);
                ASSERT_TRUE(agent_tcp4_->start());
                break;
            case Transport::TCP_IPV6_TRANSPORT:
                agent_tcp6_.reset(new eprosima::uxr::TCPv6Agent(port, middleware_));
                agent_tcp6_->set_verbose_level(6);
                ASSERT_TRUE(agent_tcp6_->start());
                break;
        }
    }

    void check_messages(std::string message, size_t number, uint8_t stream_id_raw)
    {
        std::thread publisher_thread(&Client::publish, &publisher_, 1, stream_id_raw, number, message);
        std::thread subscriber_thread(&Client::subscribe, &subscriber_, 1, stream_id_raw, number, message);

        publisher_thread.join();
        subscriber_thread.join();
    }

protected:
    Transport transport_;
    std::unique_ptr<eprosima::uxr::UDPv4Agent> agent_udp4_;
    std::unique_ptr<eprosima::uxr::UDPv6Agent> agent_udp6_;
    std::unique_ptr<eprosima::uxr::TCPv4Agent> agent_tcp4_;
    std::unique_ptr<eprosima::uxr::TCPv6Agent> agent_tcp6_;
    eprosima::uxr::Middleware::Kind middleware_;
    Client publisher_;
    Client subscriber_;
    static const std::string SMALL_MESSAGE;
};

class PublisherSubscriberLost : public PublisherSubscriberNoLost {};

const std::string PublisherSubscriberNoLost::SMALL_MESSAGE("Hello DDS world!");

TEST_P(PublisherSubscriberNoLost, PubSub1TopicsBestEffort)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 1, 0x01);
}

TEST_P(PublisherSubscriberNoLost, PubSub10TopicsBestEffort)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 10, 0x01);
}

TEST_P(PublisherSubscriberNoLost, PubSub1TopicsReliable)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 1, 0x80);
}

TEST_P(PublisherSubscriberNoLost, PubSub10TopicsReliable)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 10, 0x80);
}

TEST_P(PublisherSubscriberNoLost, PubSub1ContinousFragmentedTopic)
{
    std::string message(size_t(publisher_.get_mtu() * 8), 'A');
    publisher_.publish(1, 0x80, 1, message);
}

// TODO (#4423) Fix the non-reliable behavior when messages is higher than the agent history to enable this
/*TEST_P(PublisherSubscriberNoLost, PubSub30TopicsReliable)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 30, 0x80);
}*/

INSTANTIATE_TEST_CASE_P(
    TransportAndLost,
    PublisherSubscriberNoLost,
    ::testing::Combine(
        ::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::TCP_IPV6_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS, MiddlewareKind::FASTRTPS, MiddlewareKind::CED),
        ::testing::Values(0.0f)));

TEST_P(PublisherSubscriberLost, PubSub1FragmentedTopic2Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 1.5), 'A');
    check_messages(message, 1, 0x80);
}

TEST_P(PublisherSubscriberLost, PubSub3FragmentedTopic2Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 1.5), 'A');
    check_messages(message, 3, 0x80);
}

TEST_P(PublisherSubscriberLost, PubSub1FragmentedTopic4Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 3.5), 'A');
    check_messages(message, 1, 0x80);
}

TEST_P(PublisherSubscriberLost, PubSub3FragmentedTopic4Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 3.5), 'A');
    check_messages(message, 3, 0x80);
}

INSTANTIATE_TEST_CASE_P(
    TransportAndLost,
    PublisherSubscriberLost,
    ::testing::Combine(
        ::testing::Values(Transport::UDP_IPV4_TRANSPORT),
        ::testing::Values(MiddlewareKind::CED),
        ::testing::Values(0.05f, 0.1f)));


int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
