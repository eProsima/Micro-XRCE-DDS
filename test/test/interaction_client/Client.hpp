#ifndef IN_TEST_CLIENT_HPP
#define IN_TEST_CLIENT_HPP

#include "BigHelloWorld.h"
#include "Gateway.hpp"

#include <uxr/client/client.h>
#include <ucdr/microcdr.h>

#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#define UDP_TRANSPORT 1
#define TCP_TRANSPORT 2

inline bool operator == (const uxrObjectId& obj1, const uxrObjectId& obj2)
{
    return obj1.id == obj2.id
        && obj1.type == obj2.type;
}

inline bool operator == (const uxrStreamId& s1, const uxrStreamId& s2)
{
    return s1.raw == s2.raw
        && s1.index == s2.index
        && s1.type == s2.type
        && s1.direction == s2.direction;
}

class Client
{
public:
    Client(float lost, uint16_t history)
    : gateway_(lost)
    , client_key_(++next_client_key_)
    , history_(history)
    {
    }

    virtual ~Client()
    {}

    void create_entities_xml(uint8_t id, uint8_t stream_id_raw, uint8_t expected_status, uint8_t flags)
    {
        uxrStreamId output_stream_id = uxr_stream_id_from_raw(stream_id_raw, UXR_OUTPUT_STREAM);
        uint16_t request_id; uint8_t status;

        uxrObjectId participant_id = uxr_object_id(id, UXR_PARTICIPANT_ID);
        request_id = uxr_buffer_create_participant_xml(&session_, output_stream_id, participant_id, 0, participant_xml_, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId topic_id = uxr_object_id(id, UXR_TOPIC_ID);
        request_id = uxr_buffer_create_topic_xml(&session_, output_stream_id, topic_id, participant_id, topic_xml_, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId publisher_id = uxr_object_id(id, UXR_PUBLISHER_ID);
        request_id = uxr_buffer_create_publisher_xml(&session_, output_stream_id, publisher_id, participant_id, publisher_xml_, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId datawriter_id = uxr_object_id(id, UXR_DATAWRITER_ID);
        request_id = uxr_buffer_create_datawriter_xml(&session_, output_stream_id, datawriter_id, publisher_id, datawriter_xml_, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId subscriber_id = uxr_object_id(id, UXR_SUBSCRIBER_ID);
        request_id = uxr_buffer_create_subscriber_xml(&session_, output_stream_id, subscriber_id, participant_id, subscriber_xml_, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId datareader_id = uxr_object_id(id, UXR_DATAREADER_ID);
        request_id = uxr_buffer_create_datareader_xml(&session_, output_stream_id, datareader_id, subscriber_id, datareader_xml_, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
    }

    void create_entities_ref(uint8_t id, uint8_t stream_id_raw, uint8_t expected_status, uint8_t flags)
    {
        uxrStreamId output_stream_id = uxr_stream_id_from_raw(stream_id_raw, UXR_OUTPUT_STREAM);
        uint16_t request_id; uint8_t status;

        uxrObjectId participant_id = uxr_object_id(id, UXR_PARTICIPANT_ID);
        request_id = uxr_buffer_create_participant_ref(&session_, output_stream_id, participant_id, 0, "default_xrce_participant", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(participant_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId topic_id = uxr_object_id(id, UXR_TOPIC_ID);
        request_id = uxr_buffer_create_topic_ref(&session_, output_stream_id, topic_id, participant_id, "bighelloworld_topic", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(topic_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId publisher_id = uxr_object_id(id, UXR_PUBLISHER_ID);
        request_id = uxr_buffer_create_publisher_xml(&session_, output_stream_id, publisher_id, participant_id, "", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(publisher_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId datawriter_id = uxr_object_id(id, UXR_DATAWRITER_ID);
        request_id = uxr_buffer_create_datawriter_ref(&session_, output_stream_id, datawriter_id, publisher_id, "bighelloworld_data_writer", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(datawriter_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId subscriber_id = uxr_object_id(id, UXR_SUBSCRIBER_ID);
        request_id = uxr_buffer_create_subscriber_xml(&session_, output_stream_id, subscriber_id, participant_id, "", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(subscriber_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId datareader_id = uxr_object_id(id, UXR_DATAREADER_ID);
        request_id = uxr_buffer_create_datareader_ref(&session_, output_stream_id, datareader_id, subscriber_id, "bighelloworld_data_reader", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(datareader_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);
    }

    void publish(uint8_t id, uint8_t stream_id_raw, size_t number, const std::string& message)
    {
        //Used only for waiting the RTPS subscriber matching
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        (void) uxr_run_session_time(&session_, 500);

        uxrStreamId output_stream_id = uxr_stream_id_from_raw(stream_id_raw, UXR_OUTPUT_STREAM);
        uxrObjectId datawriter_id = uxr_object_id(id, UXR_DATAWRITER_ID);

        for(size_t i = 0; i < number; ++i)
        {
            BigHelloWorld topic;
            topic.index = static_cast<uint32_t>(i);
            strcpy(topic.message, message.c_str());

            ucdrBuffer ub;
            uint32_t topic_size = BigHelloWorld_size_of_topic(&topic, 0);
            bool prepared = uxr_prepare_output_stream(&session_, output_stream_id, datawriter_id, &ub, topic_size);
            ASSERT_TRUE(prepared);
            bool written = BigHelloWorld_serialize_topic(&ub, &topic);
            ASSERT_TRUE(written);
            ASSERT_FALSE(ub.error);
            bool sent = uxr_run_session_until_confirm_delivery(&session_, 3000);
            ASSERT_TRUE(sent);
            std::cout << "topic sent: " << i << std::endl;
        }
    }

    void subscribe(uint8_t id, uint8_t stream_id_raw, size_t number, const std::string& message)
    {
        //Used only for waiting the RTPS publisher matching
        std::this_thread::sleep_for(std::chrono::milliseconds(2600));

        expected_message_ = message;
        expected_topic_index_ = 0;
        last_topic_stream_id_ = uxr_stream_id_from_raw(0, UXR_OUTPUT_STREAM);
        last_topic_object_id_ = uxr_object_id(255, 15);

        uxrStreamId output_stream_id = uxr_stream_id(0, UXR_RELIABLE_STREAM, UXR_OUTPUT_STREAM);
        uxrStreamId input_stream_id = uxr_stream_id_from_raw(stream_id_raw, UXR_INPUT_STREAM);
        uxrObjectId datareader_id = uxr_object_id(id, UXR_DATAREADER_ID);

        uxrDeliveryControl delivery_control = {};
        delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
        uint16_t request_id = uxr_buffer_request_data(&session_, output_stream_id, datareader_id, input_stream_id, &delivery_control);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);

        while(expected_topic_index_ < number)
        {
            uint8_t status;
            bool received_ok = uxr_run_session_until_all_status(&session_, 3000, &request_id, &status, 1);
            ASSERT_EQ(UXR_STATUS_OK, status);
            ASSERT_TRUE(received_ok);
            ASSERT_EQ(last_topic_object_id_, datareader_id);
            ASSERT_EQ(last_topic_stream_id_, input_stream_id);
            ASSERT_EQ(last_topic_request_id_, request_id);

            ASSERT_EQ(UXR_STATUS_OK, last_status_);
            ASSERT_EQ(datareader_id, last_status_object_id_);
            ASSERT_EQ(request_id, last_status_request_id_);
        }
    }

    void init_transport(int transport, const char* ip, uint16_t port)
    {
        switch(transport)
        {
            case UDP_TRANSPORT:
                mtu_ = UXR_CONFIG_UDP_TRANSPORT_MTU;
                ASSERT_TRUE(uxr_init_udp_transport(&udp_transport_, &udp_platform_, ip, port));
                uxr_init_session(&session_, gateway_.monitorize(&udp_transport_.comm), client_key_);
                break;
            case TCP_TRANSPORT:
                mtu_ = UXR_CONFIG_TCP_TRANSPORT_MTU;
                ASSERT_TRUE(uxr_init_tcp_transport(&tcp_transport_, &tcp_platform_, ip, port));
                uxr_init_session(&session_, gateway_.monitorize(&tcp_transport_.comm), client_key_);
                break;
        }

        init_common();
    }

    void close_transport(int transport)
    {
        // Flash incomming messages.
        uxr_run_session_time(&session_, 1000);

        bool deleted = uxr_delete_session(&session_);

        if(0.0f == gateway_.get_lost_value()) //because the agent only send one status to a delete in stream 0.
        {
            EXPECT_TRUE(deleted);
            EXPECT_EQ(UXR_STATUS_OK, session_.info.last_requested_status);
        }

        switch(transport)
        {
            case UDP_TRANSPORT:
                ASSERT_TRUE(uxr_close_udp_transport(&udp_transport_));
                break;
            case TCP_TRANSPORT:
                ASSERT_TRUE(uxr_close_tcp_transport(&tcp_transport_));
                break;
        }
    }

    size_t get_mtu() const
    {
        return mtu_;
    }

private:
    void init_common()
    {
        /* Setup callback. */
        uxr_set_topic_callback(&session_, on_topic_dispatcher, this);
        uxr_set_status_callback(&session_, on_status_dispatcher, this);

        /* Create session. */
        ASSERT_TRUE(uxr_create_session(&session_));
        ASSERT_EQ(UXR_STATUS_OK, session_.info.last_requested_status);

        /* Setup streams. */
        output_best_effort_stream_buffer_.reset(new uint8_t[mtu_ * UXR_CONFIG_MAX_OUTPUT_BEST_EFFORT_STREAMS]{0});
        output_reliable_stream_buffer_.reset(new uint8_t[mtu_ * history_ * UXR_CONFIG_MAX_OUTPUT_RELIABLE_STREAMS]{0});
        input_reliable_stream_buffer_.reset(new uint8_t[mtu_ * history_ * UXR_CONFIG_MAX_INPUT_RELIABLE_STREAMS]{0});
        for(size_t i = 0; i < UXR_CONFIG_MAX_OUTPUT_BEST_EFFORT_STREAMS; ++i)
        {
            uint8_t* buffer = output_best_effort_stream_buffer_.get() + mtu_ * i;
            (void) uxr_create_output_best_effort_stream(&session_, buffer, mtu_);
        }
        for(size_t i = 0; i < UXR_CONFIG_MAX_INPUT_BEST_EFFORT_STREAMS; ++i)
        {
            (void) uxr_create_input_best_effort_stream(&session_);
        }
        for(size_t i = 0; i < UXR_CONFIG_MAX_OUTPUT_RELIABLE_STREAMS; ++i)
        {
            uint8_t* buffer = output_reliable_stream_buffer_.get() + mtu_ * history_ * i;
            (void) uxr_create_output_reliable_stream(&session_, buffer , mtu_ * history_, history_);
        }
        for(size_t i = 0; i < UXR_CONFIG_MAX_INPUT_RELIABLE_STREAMS; ++i)
        {
            uint8_t* buffer = input_reliable_stream_buffer_.get() + mtu_ * history_ * i;
            (void) uxr_create_input_reliable_stream(&session_, buffer, mtu_ * history_, history_);
        }
    }

    static void on_topic_dispatcher(uxrSession* session_, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* serialization, void* args)
    {
        static_cast<Client*>(args)->on_topic(session_, object_id, request_id, stream_id, serialization);
    }

    void on_topic(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* serialization)
    {
        (void) session;

        BigHelloWorld topic;
        BigHelloWorld_deserialize_topic(serialization, &topic);

        ASSERT_EQ(expected_topic_index_, topic.index);
        ASSERT_STREQ(expected_message_.c_str(), topic.message);
        last_topic_object_id_ = object_id;
        last_topic_stream_id_ = stream_id;
        last_topic_request_id_ = request_id;
        expected_topic_index_++;

        std::cout << "topic received: " << topic.index << std::endl;
    }

    static void on_status_dispatcher(uxrSession* session_, uxrObjectId object_id, uint16_t request_id, uint8_t status, void* args)
    {
        static_cast<Client*>(args)->on_status(session_, object_id, request_id, status);
    }

    void on_status(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uint8_t status)
    {
        (void) session;

        last_status_ = status;
        last_status_object_id_ = object_id;
        last_status_request_id_ = request_id;
    }

    static uint32_t next_client_key_;
    static const char* participant_xml_;
    static const char* topic_xml_;
    static const char* publisher_xml_;
    static const char* subscriber_xml_;
    static const char* datawriter_xml_;
    static const char* datareader_xml_;

    Gateway gateway_;

    uint32_t client_key_;
    uint16_t history_;

    uxrUDPTransport udp_transport_;
    uxrUDPPlatform udp_platform_;
    uxrTCPTransport tcp_transport_;
    uxrTCPPlatform tcp_platform_;

    size_t mtu_;
    uxrSession session_;

    std::unique_ptr<uint8_t[]> output_best_effort_stream_buffer_;
    std::unique_ptr<uint8_t[]> output_reliable_stream_buffer_;
    std::unique_ptr<uint8_t[]> input_reliable_stream_buffer_;

    std::string expected_message_;

    uint8_t last_status_;
    uxrObjectId last_status_object_id_;
    uint16_t last_status_request_id_;

    uxrObjectId last_topic_object_id_;
    uxrStreamId last_topic_stream_id_;
    uint16_t last_topic_request_id_;
    size_t expected_topic_index_;
};


#endif //IN_TEST_CLIENT_HPP
