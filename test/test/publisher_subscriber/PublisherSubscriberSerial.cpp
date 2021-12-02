#include <gtest/gtest.h>
#include <thread>

#include <ClientSerial.hpp>
#include "../client_agent/ClientAgentSerial.hpp"

class PubSubSerial : public ClientSerial
{
public:
    PubSubSerial(std::tuple<Transport, MiddlewareKind, float, XRCECreationMode> parameters,
          uint8_t id,
          int masterfd)
        : ClientSerial(std::get<2>(parameters), 8)
        , transport_(std::get<0>(parameters))
        , middleware_(std::get<1>(parameters))
        , creation_mode_(std::get<3>(parameters))
        , id_(id)
        , masterfd_(masterfd)
    {}

    ~PubSubSerial()
    {}

    void init()
    {
        switch(transport_)
        {
            case Transport::SERIAL_TRANSPORT:
            case Transport::MULTISERIAL_TRANSPORT:
                grantpt(masterfd_);
                unlockpt(masterfd_);
                ASSERT_NO_FATAL_FAILURE(ClientSerial::init_transport(transport_, ptsname(masterfd_), NULL));
                break;

            default:
                FAIL() << "Transport type not supported";
                break;
        }

        if (creation_mode_ == XRCECreationMode::XRCE_XML_CREATION)
        {
            switch (middleware_)
            {
                case MiddlewareKind::FASTDDS:
                    ASSERT_NO_FATAL_FAILURE(ClientSerial::create_entities_xml<MiddlewareKind::FASTDDS>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::FASTRTPS:
                    ASSERT_NO_FATAL_FAILURE(ClientSerial::create_entities_xml<MiddlewareKind::FASTRTPS>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::CED:
                    ASSERT_NO_FATAL_FAILURE(ClientSerial::create_entities_xml<MiddlewareKind::CED>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
            }
        }
        else if (creation_mode_ == XRCECreationMode::XRCE_BIN_CREATION)
        {
            switch (middleware_)
            {
                case MiddlewareKind::FASTDDS:
                    ASSERT_NO_FATAL_FAILURE(ClientSerial::create_entities_bin<MiddlewareKind::FASTDDS>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                case MiddlewareKind::CED:
                    ASSERT_NO_FATAL_FAILURE(ClientSerial::create_entities_bin<MiddlewareKind::CED>(id_, 0x80, UXR_STATUS_OK, 0));
                    break;
                default:
                    FAIL() << "Creation mode not supported";
                    break;
            }
        }
    }

    void close()
    {
        ASSERT_NO_FATAL_FAILURE(ClientSerial::close_transport(transport_));
    }

    uint8_t id_;

private:
    Transport transport_;
    MiddlewareKind middleware_;
    XRCECreationMode creation_mode_;
    int masterfd_;
};


class PublisherSubscriberSerial : public ::testing::TestWithParam<std::tuple<Transport, MiddlewareKind, float, XRCECreationMode>>
{
public:
    PublisherSubscriberSerial()
        : transport_(std::get<0>(GetParam()))
        , agent_(transport_, (MiddlewareKind) std::get<1>(GetParam()))
        , clients_{}
    {
        agent_.start();
        agent_.wait_multiserial_open();

        for (auto & element : agent_.getfd_multi())
        {
            PubSubSerial entity(GetParam(), element, element);
            clients_.push_back(entity);
        }
    }

    ~PublisherSubscriberSerial()
    {}

    void SetUp() override
    {
        for (auto & entity : clients_)
        {
            entity.init();
        }
    }

    void TearDown() override
    {
        for (auto & entity : clients_)
        {
            ASSERT_NO_FATAL_FAILURE(entity.close());
        }
    }

    void check_messages(std::string message, size_t number, uint8_t stream_id_raw)
    {
        int expected_number = (clients_.size()-1)*number;
        std::thread subscriber_thread(&ClientSerial::subscribe, &clients_[0],  clients_[0].id_, stream_id_raw, expected_number, message);
        std::vector<std::thread> pub_thr;

        for (auto it = clients_.begin()+1; it != clients_.end(); it++)
        {
            std::thread publisher_thread(&ClientSerial::publish, it, it->id_, stream_id_raw, number, message);
            pub_thr.push_back(std::move(publisher_thread));
        }

        for (auto & thr : pub_thr)
        {
            if (thr.joinable())
            {
                thr.join();
            }
        }

        if (subscriber_thread.joinable())
        {
            subscriber_thread.join();
        }
    }

protected:
    Transport transport_;
    AgentSerial agent_;
    std::vector<PubSubSerial> clients_;
    static const std::string SMALL_MESSAGE;
};

const std::string PublisherSubscriberSerial::SMALL_MESSAGE("Hello DDS world!");

TEST_P(PublisherSubscriberSerial, MultiPubSub)
{
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Waiting for matching.
    check_messages(SMALL_MESSAGE, 10, 0x01);
}

INSTANTIATE_TEST_CASE_P(
    MultiSerialPubSubBin,
    PublisherSubscriberSerial,
    ::testing::Combine(
        ::testing::Values(Transport::MULTISERIAL_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS),
        ::testing::Values(0.0f),
        ::testing::Values(XRCECreationMode::XRCE_BIN_CREATION)));
