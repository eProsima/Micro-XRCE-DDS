#include <gtest/gtest.h>

#include <Client.hpp>
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


#include <thread>

class PublisherSubscriberInteraction : public ::testing::TestWithParam<std::tuple<Transport, float>>
{
public:
    const char* AGENT_PORT = "2018";

    PublisherSubscriberInteraction()
    : transport_(std::get<0>(GetParam()))
    , publisher_(std::get<1>(GetParam()), 8)
    , subscriber_(std::get<1>(GetParam()), 8)
    {
        init_agent(std::stoi(AGENT_PORT));
    }

    ~PublisherSubscriberInteraction()
    {}

    void SetUp() override
    {
        ASSERT_NO_FATAL_FAILURE(publisher_.init_transport(transport_, "127.0.0.1", AGENT_PORT));
        ASSERT_NO_FATAL_FAILURE(subscriber_.init_transport(transport_, "127.0.0.1", AGENT_PORT));

        ASSERT_NO_FATAL_FAILURE(publisher_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
        ASSERT_NO_FATAL_FAILURE(subscriber_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
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
                agent_.reset(new eprosima::uxr::UDPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                break;
            case Transport::UDP_IPV6_TRANSPORT:
                // agent_.reset(new eprosima::uxr::UDPv6Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                // break;
            case Transport::TCP_IPV4_TRANSPORT:
                // agent_.reset(new eprosima::uxr::TCPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST));
                // break;
            case Transport::TCP_IPV6_TRANSPORT:
        //         agent_.reset(new eprosima::uxr::v4Agent(port, eprosima::uxr::Middleware::Kind::FAST));
        //         break;
                break;
        }
        agent_->run();
        agent_->set_verbose_level(6);
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
    std::unique_ptr<eprosima::uxr::UDPv4Agent> agent_;
    Client publisher_;
    Client subscriber_;
    static const std::string SMALL_MESSAGE;
};

const std::string PublisherSubscriberInteraction::SMALL_MESSAGE("Hello DDS world!");

INSTANTIATE_TEST_CASE_P(TransportAndLost, PublisherSubscriberInteraction,
        ::testing::Combine(::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::TCP_IPV6_TRANSPORT), ::testing::Values(0.0f, 0.05f, 0.1f)));

TEST_P(PublisherSubscriberInteraction, PubSub1TopicsBestEffort)
{
    if(0.0f == std::get<1>(GetParam())) //only without lost
    {
        check_messages(SMALL_MESSAGE, 1, 0x01);
    }
}

TEST_P(PublisherSubscriberInteraction, PubSub10TopicsBestEffort)
{
    if(0.0f == std::get<1>(GetParam())) //only without lost
    {
        check_messages(SMALL_MESSAGE, 10, 0x01);
    }
}

TEST_P(PublisherSubscriberInteraction, PubSub1TopicsReliable)
{
    check_messages(SMALL_MESSAGE, 1, 0x80);
}

TEST_P(PublisherSubscriberInteraction, PubSub10TopicsReliable)
{
    check_messages(SMALL_MESSAGE, 10, 0x80);
}

// TODO (#4423) Fix the non-reliable behavior when messages is higher than the agent history to enable this
/*TEST_P(PublisherSubscriberInteraction, PubSub30TopicsReliable)
{
    check_messages(SMALL_MESSAGE, 30, 0x80);
}
*/

TEST_P(PublisherSubscriberInteraction, PubSub1FragmentedTopic2Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 1.5), 'A');
    check_messages(message, 1, 0x80);
}

TEST_P(PublisherSubscriberInteraction, PubSub3FragmentedTopic2Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 1.5), 'A');
    check_messages(message, 3, 0x80);
}

TEST_P(PublisherSubscriberInteraction, PubSub1FragmentedTopic4Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 3.5), 'A');
    check_messages(message, 1, 0x80);
}

TEST_P(PublisherSubscriberInteraction, PubSub3FragmentedTopic4Parts)
{
    std::string message(size_t(publisher_.get_mtu() * 3.5), 'A');
    check_messages(message, 3, 0x80);
}

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
