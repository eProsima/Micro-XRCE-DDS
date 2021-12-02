#include <gtest/gtest.h>
#include <thread>

#include <ClientCan.hpp>
#include "../client_agent/ClientAgentCan.hpp"

class PubSubCan : public ClientCan
{
public:
    PubSubCan(std::tuple<MiddlewareKind, float, XRCECreationMode> parameters,
          uint8_t id,
          const char *dev,
          const uint32_t can_id)
        : ClientCan(std::get<1>(parameters), 16)
        , middleware_(std::get<0>(parameters))
        , creation_mode_(std::get<2>(parameters))
        , id_(id)
        , dev_(dev)
        , can_id_(can_id)
    {}

    ~PubSubCan()
    {}

    void init()
    {
        ASSERT_NO_FATAL_FAILURE(ClientCan::init_transport(dev_, can_id_));

        if (creation_mode_ == XRCECreationMode::XRCE_XML_CREATION)
        {
            switch (middleware_)
            {
                case MiddlewareKind::FASTDDS:
                    ASSERT_NO_FATAL_FAILURE(ClientCan::create_entities_xml<MiddlewareKind::FASTDDS>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::FASTRTPS:
                    ASSERT_NO_FATAL_FAILURE(ClientCan::create_entities_xml<MiddlewareKind::FASTRTPS>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::CED:
                    ASSERT_NO_FATAL_FAILURE(ClientCan::create_entities_xml<MiddlewareKind::CED>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
            }
        }
        else if (creation_mode_ == XRCECreationMode::XRCE_BIN_CREATION)
        {
            switch (middleware_)
            {
                case MiddlewareKind::FASTDDS:
                    ASSERT_NO_FATAL_FAILURE(ClientCan::create_entities_bin<MiddlewareKind::FASTDDS>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::CED:
                    ASSERT_NO_FATAL_FAILURE(ClientCan::create_entities_bin<MiddlewareKind::CED>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                default:
                    FAIL() << "Transport type not supported";
                    break;
            }
        }
    }

    void close()
    {
        ASSERT_NO_FATAL_FAILURE(ClientCan::close_transport());
    }

    uint8_t id_;

private:
    MiddlewareKind middleware_;
    XRCECreationMode creation_mode_;
    const char *dev_;
    const uint32_t can_id_;
};


class PublisherSubscriberCan : public ::testing::TestWithParam<std::tuple<MiddlewareKind, float, XRCECreationMode>>
{
public:
    PublisherSubscriberCan()
        : agent_((MiddlewareKind) std::get<1>(GetParam()))
        , publisher_(GetParam(), 1, agent_.dev, agent_.can_id + 1)
        , subscriber_(GetParam(), 1, agent_.dev, agent_.can_id + 2)
    {
        agent_.start();
    }

    ~PublisherSubscriberCan()
    {
        agent_.stop();
    }

    void SetUp() override
    {
        ASSERT_NO_FATAL_FAILURE(publisher_.init());
        ASSERT_NO_FATAL_FAILURE(subscriber_.init());
    }

    void TearDown() override
    {
        ASSERT_NO_FATAL_FAILURE(publisher_.close());
        ASSERT_NO_FATAL_FAILURE(subscriber_.close());
    }

    void check_messages(std::string message, size_t number, uint8_t stream_id_raw)
    {
        std::thread publisher_thread(&ClientCan::publish, &publisher_, publisher_.id_, stream_id_raw, number, message);
        std::thread subscriber_thread(&ClientCan::subscribe, &subscriber_, subscriber_.id_, stream_id_raw, number, message);

        publisher_thread.join();
        subscriber_thread.join();
    }

protected:
    AgentCan agent_;
    PubSubCan publisher_;
    PubSubCan subscriber_;
    static const std::string SMALL_MESSAGE;
};

const std::string PublisherSubscriberCan::SMALL_MESSAGE("Hello DDS world!");

TEST_P(PublisherSubscriberCan, PubSub10TopicsBestEffort)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 10, 0x01);
}

TEST_P(PublisherSubscriberCan, PubSub10TopicsReliable)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 10, 0x80);
}

TEST_P(PublisherSubscriberCan, PubSub1ContinousFragmentedTopic)
{
    std::string message(size_t(publisher_.get_mtu() * 8), 'A');
    publisher_.publish(1, 0x80, 1, message);
}

INSTANTIATE_TEST_SUITE_P(
    PubSubCan,
    PublisherSubscriberCan,
    ::testing::Combine(
        ::testing::Values(MiddlewareKind::FASTDDS),
        ::testing::Values(0.0f),
        ::testing::Values(XRCECreationMode::XRCE_XML_CREATION, XRCECreationMode::XRCE_BIN_CREATION)));