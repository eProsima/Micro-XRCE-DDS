#include <gtest/gtest.h>
#include <thread>

#include <Client.hpp>
#include "../client_agent/ClientAgentInteraction.hpp"

class PubSub : public Client
{
public:
    PubSub(std::tuple<Transport, MiddlewareKind, float, XRCECreationMode> parameters,
          const uint16_t AGENT_PORT,
          uint8_t id)
        : Client(std::get<2>(parameters), 8)
        , transport_(std::get<0>(parameters))
        , middleware_(std::get<1>(parameters))
        , creation_mode_(std::get<3>(parameters))
        , AGENT_PORT_(AGENT_PORT)
        , id_(id)
    {        
    }

    ~PubSub()
    {}

    void init()
    {
        switch(transport_)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            case Transport::TCP_IPV4_TRANSPORT:
            {
                ASSERT_NO_FATAL_FAILURE(Client::init_transport(transport_, "127.0.0.1", std::to_string(AGENT_PORT_).c_str()));
                break;
            }
            case Transport::UDP_IPV6_TRANSPORT:
            case Transport::TCP_IPV6_TRANSPORT:
            {
                ASSERT_NO_FATAL_FAILURE(Client::init_transport(transport_, "::1", std::to_string(AGENT_PORT_).c_str()));
                break;
            }
            
            case Transport::CUSTOM_WITH_FRAMING:
            case Transport::CUSTOM_WITHOUT_FRAMING:
            {
                ASSERT_NO_FATAL_FAILURE(Client::init_transport(transport_, NULL, NULL));
                break;
            }
        }

        if (creation_mode_ == XRCECreationMode::XRCE_XML_CREATION)
        {
            switch (middleware_)
            {
                case MiddlewareKind::FASTDDS:
                    ASSERT_NO_FATAL_FAILURE(Client::create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::FASTRTPS:
                    ASSERT_NO_FATAL_FAILURE(Client::create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::CED:
                    ASSERT_NO_FATAL_FAILURE(Client::create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
                    break;
            }
        }
        else if (creation_mode_ == XRCECreationMode::XRCE_BIN_CREATION)
        {
            switch (middleware_)
            {
                case MiddlewareKind::FASTDDS:
                    ASSERT_NO_FATAL_FAILURE(Client::create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
                    break;
                default:
                    // Not supported
                    ASSERT_TRUE(0);
                    break;
            }
        }
    }

    void close()
    {
        ASSERT_NO_FATAL_FAILURE(Client::close_transport(transport_));
    }

private:
    const uint16_t AGENT_PORT_;
    Transport transport_;
    MiddlewareKind middleware_;
    XRCECreationMode creation_mode_;
    uint8_t id_;
};

class PublisherSubscriberNoLost : public ::testing::TestWithParam<std::tuple<Transport, MiddlewareKind, float, XRCECreationMode>>
{
public:
    const uint16_t AGENT_PORT = 2018 + uint16_t(std::get<0>(this->GetParam()));

    PublisherSubscriberNoLost()
        : transport_(std::get<0>(GetParam()))
        , agent_(transport_, (MiddlewareKind) std::get<1>(GetParam()), AGENT_PORT)
        , publisher_(GetParam(), AGENT_PORT, 1)
        , subscriber_(GetParam(), AGENT_PORT, 1)
    {        
        agent_.start();
    }

    ~PublisherSubscriberNoLost()
    {}

    void SetUp() override
    {
        publisher_.init();
        subscriber_.init();
    }

    void TearDown() override
    {
        ASSERT_NO_FATAL_FAILURE(publisher_.close());
        ASSERT_NO_FATAL_FAILURE(subscriber_.close());
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
    Agent agent_;
    PubSub publisher_;
    PubSub subscriber_;
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
        ::testing::Values(0.0f),
        ::testing::Values(XRCECreationMode::XRCE_XML_CREATION)));

INSTANTIATE_TEST_CASE_P(
    TransportAndLostCreationModes,
    PublisherSubscriberNoLost,
    ::testing::Combine(
        ::testing::Values(Transport::UDP_IPV4_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS),
        ::testing::Values(0.0f),
        ::testing::Values(XRCECreationMode::XRCE_XML_CREATION, XRCECreationMode::XRCE_BIN_CREATION)));

INSTANTIATE_TEST_CASE_P(
    TransportAndLostCustomTransports,
    PublisherSubscriberNoLost,
    ::testing::Combine(
        ::testing::Values(Transport::CUSTOM_WITH_FRAMING, Transport::CUSTOM_WITHOUT_FRAMING),
        ::testing::Values(MiddlewareKind::FASTDDS),
        ::testing::Values(0.0f),
        ::testing::Values(XRCECreationMode::XRCE_XML_CREATION)));

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
        ::testing::Values(0.05f, 0.1f),
        ::testing::Values(XRCECreationMode::XRCE_XML_CREATION)));


int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
