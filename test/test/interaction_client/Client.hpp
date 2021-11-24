#ifndef IN_TEST_CLIENT_HPP
#define IN_TEST_CLIENT_HPP

#include "BigHelloWorld.h"
#include "Gateway.hpp"
#include <EntitiesInfo.hpp>
#include <../custom_transports/Custom_transports.hpp>

#include <uxr/client/client.h>
#include <uxr/client/util/ping.h>
#include <ucdr/microcdr.h>

#include <gtest/gtest.h>
#include <iostream>
#include <thread>

enum class Transport
{
    UDP_IPV4_TRANSPORT,
    UDP_IPV6_TRANSPORT,
    TCP_IPV4_TRANSPORT,
    TCP_IPV6_TRANSPORT,
    CAN_TRANSPORT,
    SERIAL_TRANSPORT,
    MULTISERIAL_TRANSPORT,
    CUSTOM_WITH_FRAMING,
    CUSTOM_WITHOUT_FRAMING
};

enum class XRCECreationMode
{
    XRCE_XML_CREATION,
    XRCE_BIN_CREATION,
    XRCE_REF_CREATION
};

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

extern "C" bool flush_session(uxrSession* session, void * args);

class Client
{
    const int timeout = 30000;
public:
    Client(float lost, uint16_t history)
    : gateway_(lost)
    , client_key_(++next_client_key_)
    , history_(history)
    {
    }

    virtual ~Client()
    {}

    template<MiddlewareKind Kind>
    void create_entities_xml(uint8_t id, uint8_t stream_id_raw, uint8_t expected_status, uint8_t flags)
    {
        using EInfo = EntitiesInfo<Kind>;

        uxrStreamId output_stream_id = uxr_stream_id_from_raw(stream_id_raw, UXR_OUTPUT_STREAM);
        uint16_t request_id; uint8_t status;

        uxrObjectId participant_id = uxr_object_id(id, UXR_PARTICIPANT_ID);
        request_id =
            uxr_buffer_create_participant_xml(
                &session_, output_stream_id, participant_id, 0, EInfo::participant_xml, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId topic_id = uxr_object_id(id, UXR_TOPIC_ID);
        request_id =
            uxr_buffer_create_topic_xml(
                &session_, output_stream_id, topic_id, participant_id, EInfo::topic_xml, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId publisher_id = uxr_object_id(id, UXR_PUBLISHER_ID);
        request_id =
            uxr_buffer_create_publisher_xml(
                &session_, output_stream_id, publisher_id, participant_id, EInfo::publisher_xml, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId datawriter_id = uxr_object_id(id, UXR_DATAWRITER_ID);
        request_id =
            uxr_buffer_create_datawriter_xml(
                &session_, output_stream_id, datawriter_id, publisher_id, EInfo::datawriter_xml, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId subscriber_id = uxr_object_id(id, UXR_SUBSCRIBER_ID);
        request_id =
            uxr_buffer_create_subscriber_xml(
                &session_, output_stream_id, subscriber_id, participant_id, EInfo::subscriber_xml, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);

        uxrObjectId datareader_id = uxr_object_id(id, UXR_DATAREADER_ID);
        request_id =
            uxr_buffer_create_datareader_xml(
                &session_, output_stream_id, datareader_id, subscriber_id, EInfo::datareader_xml, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
    }

    template<MiddlewareKind Kind>
    void create_entities_ref(uint8_t id, uint8_t stream_id_raw, uint8_t expected_status, uint8_t flags)
    {
        using EInfo = EntitiesInfo<Kind>;

        uxrStreamId output_stream_id = uxr_stream_id_from_raw(stream_id_raw, UXR_OUTPUT_STREAM);
        uint16_t request_id; uint8_t status;

        uxrObjectId participant_id = uxr_object_id(id, UXR_PARTICIPANT_ID);
        request_id =
            uxr_buffer_create_participant_ref(
                &session_, output_stream_id, participant_id, 0, EInfo::participant_ref, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(participant_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId topic_id = uxr_object_id(id, UXR_TOPIC_ID);
        request_id =
            uxr_buffer_create_topic_ref(
                &session_, output_stream_id, topic_id, participant_id, EInfo::topic_ref, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(topic_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId publisher_id = uxr_object_id(id, UXR_PUBLISHER_ID);
        request_id =
            uxr_buffer_create_publisher_xml(
                &session_, output_stream_id, publisher_id, participant_id, EInfo::publisher_ref, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(publisher_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId datawriter_id = uxr_object_id(id, UXR_DATAWRITER_ID);
        request_id =
            uxr_buffer_create_datawriter_ref(
                &session_, output_stream_id, datawriter_id, publisher_id, EInfo::datawriter_ref, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(datawriter_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId subscriber_id = uxr_object_id(id, UXR_SUBSCRIBER_ID);
        request_id =
            uxr_buffer_create_subscriber_xml(
                &session_, output_stream_id, subscriber_id, participant_id, EInfo::subscriber_ref, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(subscriber_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId datareader_id = uxr_object_id(id, UXR_DATAREADER_ID);
        request_id =
            uxr_buffer_create_datareader_ref(
                &session_, output_stream_id, datareader_id, subscriber_id, EInfo::datareader_ref, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(datareader_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);
    }

    template<MiddlewareKind Kind>
    void create_entities_bin(uint8_t id, uint8_t stream_id_raw, uint8_t expected_status, uint8_t flags)
    {
        uxrStreamId output_stream_id = uxr_stream_id_from_raw(stream_id_raw, UXR_OUTPUT_STREAM);
        uint16_t request_id; uint8_t status;

        uxrObjectId participant_id = uxr_object_id(id, UXR_PARTICIPANT_ID);
        request_id =
            uxr_buffer_create_participant_bin(
                &session_, output_stream_id, participant_id, 0, "participant_name", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(participant_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId topic_id = uxr_object_id(id, UXR_TOPIC_ID);
        request_id =
            uxr_buffer_create_topic_bin(
                &session_, output_stream_id, topic_id, participant_id, "topicname", "topictype", flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(topic_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId publisher_id = uxr_object_id(id, UXR_PUBLISHER_ID);
        request_id =
            uxr_buffer_create_publisher_bin(
                &session_, output_stream_id, publisher_id, participant_id, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(publisher_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId datawriter_id = uxr_object_id(id, UXR_DATAWRITER_ID);
        request_id =
            uxr_buffer_create_datawriter_bin(
                &session_, output_stream_id, datawriter_id, publisher_id, topic_id, 1, 0, 0, UXR_DURABILITY_TRANSIENT_LOCAL, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(datawriter_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId subscriber_id = uxr_object_id(id, UXR_SUBSCRIBER_ID);
        request_id =
            uxr_buffer_create_subscriber_bin(
                &session_, output_stream_id, subscriber_id, participant_id, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
        ASSERT_EQ(expected_status, status);
        ASSERT_EQ(expected_status, last_status_);
        ASSERT_EQ(subscriber_id, last_status_object_id_);
        ASSERT_EQ(request_id, last_status_request_id_);

        uxrObjectId datareader_id = uxr_object_id(id, UXR_DATAREADER_ID);
        request_id =
            uxr_buffer_create_datareader_bin(
                &session_, output_stream_id, datareader_id, subscriber_id, topic_id, 1, 0, 0, UXR_DURABILITY_TRANSIENT_LOCAL, flags);
        ASSERT_NE(UXR_INVALID_REQUEST_ID, request_id);
        uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
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
            uint16_t prepared = false;
            if (topic_size < mtu_)
            {
                prepared = uxr_prepare_output_stream(&session_, output_stream_id, datawriter_id, &ub, topic_size);
            } else {
                prepared = uxr_prepare_output_stream_fragmented(&session_, output_stream_id, datawriter_id, &ub, topic_size, flush_session, NULL);
            }
            ASSERT_NE(prepared, UXR_INVALID_REQUEST_ID);
            bool written = BigHelloWorld_serialize_topic(&ub, &topic);
            ASSERT_TRUE(written);
            ASSERT_FALSE(ub.error);
            bool sent = uxr_run_session_until_confirm_delivery(&session_, timeout);
            ASSERT_TRUE(sent);
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
            bool received_ok = uxr_run_session_until_all_status(&session_, timeout, &request_id, &status, 1);
            ASSERT_EQ(UXR_STATUS_OK, status);
            ASSERT_TRUE(received_ok);
            ASSERT_EQ(last_topic_object_id_, datareader_id);
            ASSERT_EQ(last_topic_stream_id_, input_stream_id);
            ASSERT_EQ(last_topic_request_id_, request_id);

            ASSERT_EQ(UXR_STATUS_OK, last_status_);
            ASSERT_EQ(datareader_id, last_status_object_id_);
            ASSERT_EQ(request_id, last_status_request_id_);
            std::cout << "msg " << expected_topic_index_ << " received." << std::endl;
        }
    }

    void request_data(uint8_t id, uint8_t stream_id_raw, const std::string& message)
    {
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

        uxr_flash_output_streams(&session_);
    }

    size_t get_received_topics()
    {
        return expected_topic_index_;
    }

    void init_transport(Transport transport, const char* ip, const char* port)
    {
        switch(transport)
        {
            case Transport::UDP_IPV4_TRANSPORT:
                mtu_ = UXR_CONFIG_UDP_TRANSPORT_MTU;
                ASSERT_TRUE(uxr_init_udp_transport(&udp_transport_, UXR_IPv4, ip, port));
                uxr_init_session(&session_, gateway_.monitorize(&udp_transport_.comm), client_key_);
                break;
            case Transport::UDP_IPV6_TRANSPORT:
                mtu_ = UXR_CONFIG_UDP_TRANSPORT_MTU;
                ASSERT_TRUE(uxr_init_udp_transport(&udp_transport_, UXR_IPv6, ip, port));
                uxr_init_session(&session_, gateway_.monitorize(&udp_transport_.comm), client_key_);
                break;
            case Transport::TCP_IPV4_TRANSPORT:
                mtu_ = UXR_CONFIG_TCP_TRANSPORT_MTU;
                ASSERT_TRUE(uxr_init_tcp_transport(&tcp_transport_, UXR_IPv4, ip, port));
                uxr_init_session(&session_, gateway_.monitorize(&tcp_transport_.comm), client_key_);
                break;
            case Transport::TCP_IPV6_TRANSPORT:
                mtu_ = UXR_CONFIG_TCP_TRANSPORT_MTU;
                ASSERT_TRUE(uxr_init_tcp_transport(&tcp_transport_, UXR_IPv6, ip, port));
                uxr_init_session(&session_, gateway_.monitorize(&tcp_transport_.comm), client_key_);
                break;
            case Transport::CUSTOM_WITHOUT_FRAMING:
                mtu_ = UXR_CONFIG_CUSTOM_TRANSPORT_MTU;

                uxr_set_custom_transport_callbacks(
                    &custom_transport_,
                    false,
                    client_custom_transport_open,
                    client_custom_transport_close,
                    client_custom_transport_write_packet,
                    client_custom_transport_read_packet);

                ASSERT_TRUE(uxr_init_custom_transport(&custom_transport_, NULL));
                uxr_init_session(&session_, gateway_.monitorize(&custom_transport_.comm), client_key_);
                break;
            case Transport::CUSTOM_WITH_FRAMING:
                mtu_ = UXR_CONFIG_CUSTOM_TRANSPORT_MTU;

                uxr_set_custom_transport_callbacks(
                    &custom_transport_,
                    true,
                    client_custom_transport_open,
                    client_custom_transport_close,
                    client_custom_transport_write_stream,
                    client_custom_transport_read_stream);

                ASSERT_TRUE(uxr_init_custom_transport(&custom_transport_, NULL));
                uxr_init_session(&session_, gateway_.monitorize(&custom_transport_.comm), client_key_);
                break;
            default:
                FAIL() << "Transport type not supported";
                break;
        }

        init_common();
    }

    void close_transport(Transport transport)
    {
        // Flash incomming messages.
        uxr_run_session_time(&session_, 100);

        bool deleted = uxr_delete_session(&session_);

        if(0.0f == gateway_.get_lost_value()) //because the agent only send one status to a delete in stream 0.
        {
            EXPECT_TRUE(deleted);
            EXPECT_EQ(UXR_STATUS_OK, session_.info.last_requested_status);
        }

        switch(transport)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            case Transport::UDP_IPV6_TRANSPORT:
                ASSERT_TRUE(uxr_close_udp_transport(&udp_transport_));
                break;
            case Transport::TCP_IPV4_TRANSPORT:
            case Transport::TCP_IPV6_TRANSPORT:
                ASSERT_TRUE(uxr_close_tcp_transport(&tcp_transport_));
                break;
            case Transport::CUSTOM_WITHOUT_FRAMING:
            case Transport::CUSTOM_WITH_FRAMING:
                ASSERT_TRUE(uxr_close_custom_transport(&custom_transport_));
                break;
            default:
                FAIL() << "Transport type not supported";
                break;
        }
    }

    size_t get_mtu() const
    {
        return mtu_;
    }

    void ping_agent(
            const Transport transport_kind)
    {
        uxrCommunication* comm(nullptr);

        switch (transport_kind)
        {
            case Transport::UDP_IPV4_TRANSPORT:
            case Transport::UDP_IPV6_TRANSPORT:
            {
                comm = &udp_transport_.comm;
                break;
            }
            case Transport::TCP_IPV4_TRANSPORT:
            case Transport::TCP_IPV6_TRANSPORT:
            {
                comm = &tcp_transport_.comm;
                break;
            }
            case Transport::CUSTOM_WITHOUT_FRAMING:
            case Transport::CUSTOM_WITH_FRAMING:
            {
                comm = &custom_transport_.comm;
                break;
            }
            default:
                FAIL() << "Transport type not supported";
                break;
        }
        ASSERT_TRUE(uxr_ping_agent_attempts(comm, 1000, 1));
    }

protected:
    void init_common()
    {
        if (session_.on_topic == NULL)
        {
            /* Setup callback. */
            uxr_set_topic_callback(&session_, on_topic_dispatcher, this);
        }

        uxr_set_status_callback(&session_, on_status_dispatcher, this);

        /* Create session. */
        ASSERT_TRUE(uxr_create_session(&session_));
        ASSERT_EQ(UXR_STATUS_OK, session_.info.last_requested_status);

        /* Setup streams. */
        output_best_effort_stream_buffer_.reset(
            new std::vector<uint8_t>(mtu_ * UXR_CONFIG_MAX_OUTPUT_BEST_EFFORT_STREAMS, 0));
        output_reliable_stream_buffer_.reset(
            new std::vector<uint8_t>(mtu_ * history_ * UXR_CONFIG_MAX_OUTPUT_RELIABLE_STREAMS, 0));
        input_reliable_stream_buffer_.reset(
            new std::vector<uint8_t>(mtu_ * history_ * UXR_CONFIG_MAX_INPUT_RELIABLE_STREAMS, 0));
        for(size_t i = 0; i < UXR_CONFIG_MAX_OUTPUT_BEST_EFFORT_STREAMS; ++i)
        {
            uint8_t* buffer = output_best_effort_stream_buffer_->data() + mtu_ * i;
            (void) uxr_create_output_best_effort_stream(&session_, buffer, mtu_);
        }
        for(size_t i = 0; i < UXR_CONFIG_MAX_INPUT_BEST_EFFORT_STREAMS; ++i)
        {
            (void) uxr_create_input_best_effort_stream(&session_);
        }
        for(size_t i = 0; i < UXR_CONFIG_MAX_OUTPUT_RELIABLE_STREAMS; ++i)
        {
            uint8_t* buffer = output_reliable_stream_buffer_->data() + mtu_ * history_ * i;
            (void) uxr_create_output_reliable_stream(&session_, buffer , mtu_ * history_, history_);
        }
        for(size_t i = 0; i < UXR_CONFIG_MAX_INPUT_RELIABLE_STREAMS; ++i)
        {
            uint8_t* buffer = input_reliable_stream_buffer_->data() + mtu_ * history_ * i;
            (void) uxr_create_input_reliable_stream(&session_, buffer, mtu_ * history_, history_);
        }
    }

    static void on_topic_dispatcher(uxrSession* session_, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* serialization, uint16_t length, void* args)
    {
        static_cast<Client*>(args)->on_topic(session_, object_id, request_id, stream_id, serialization, length);
    }

    static void on_topic_multi_dispatcher(uxrSession* session_, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* serialization, uint16_t length, void* args)
    {
        static_cast<Client*>(args)->on_topic_multi(session_, object_id, request_id, stream_id, serialization, length);
    }

    void on_topic(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* serialization, uint16_t length)
    {
        (void) session;
        (void) length;

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

    void on_topic_multi(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* serialization, uint16_t length)
    {
        (void) session;
        (void) length;

        BigHelloWorld topic;
        BigHelloWorld_deserialize_topic(serialization, &topic);

        ASSERT_STREQ(expected_message_.c_str(), topic.message);
        last_topic_object_id_ = object_id;
        last_topic_stream_id_ = stream_id;
        last_topic_request_id_ = request_id;
        expected_topic_index_++;
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

    Gateway gateway_;

    uint32_t client_key_;
    uint16_t history_;

    uxrUDPTransport udp_transport_;
    uxrTCPTransport tcp_transport_;
    uxrCustomTransport custom_transport_;

    size_t mtu_;
    uxrSession session_;

    std::shared_ptr<std::vector<uint8_t>> output_best_effort_stream_buffer_;
    std::shared_ptr<std::vector<uint8_t>> output_reliable_stream_buffer_;
    std::shared_ptr<std::vector<uint8_t>> input_reliable_stream_buffer_;

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
