#ifndef IN_TEST_CUSTOM_TRANSPORT_HPP
#define IN_TEST_CUSTOM_TRANSPORT_HPP

#include <uxr/agent/transport/custom/CustomAgent.hpp>
#include <uxr/client/profile/transport/custom/custom_transport.h>

// Agent custom transports
extern eprosima::uxr::CustomAgent::InitFunction agent_custom_transport_open;
extern eprosima::uxr::CustomAgent::FiniFunction agent_custom_transport_close;
extern eprosima::uxr::CustomAgent::RecvMsgFunction agent_custom_transport_read_stream;
extern eprosima::uxr::CustomAgent::SendMsgFunction agent_custom_transport_write_stream;
extern eprosima::uxr::CustomAgent::RecvMsgFunction agent_custom_transport_read_packet;
extern eprosima::uxr::CustomAgent::SendMsgFunction agent_custom_transport_write_packet;

// Client custom transport
extern "C"
{
    bool client_custom_transport_open(uxrCustomTransport* transport);
    bool client_custom_transport_close(uxrCustomTransport* transport);
    size_t client_custom_transport_write_stream( uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* errcode);
    size_t client_custom_transport_read_stream( uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* errcode);
    size_t client_custom_transport_write_packet( uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* errcode);
    size_t client_custom_transport_read_packet( uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* errcode);
}

#endif //IN_TEST_CUSTOM_TRANSPORT_HPP
