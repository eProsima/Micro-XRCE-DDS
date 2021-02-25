#include "Custom_transports.hpp"
#include <uxr/client/util/time.h>

#include <queue>

using packet_fifo = std::queue<std::vector<uint8_t>>;
using stream_fifo = std::queue<uint8_t>;

static std::map<int32_t, packet_fifo> client_to_agent_packet_queue;
static std::map<int32_t, packet_fifo> agent_to_client_packet_queue;
static std::map<int32_t, stream_fifo> client_to_agent_stream_queue;
static std::map<int32_t, stream_fifo> agent_to_client_stream_queue;

std::mutex client_to_agent_mtx;
std::mutex agent_to_client_mtx;

template <class T>
static int32_t find_queue_with_data(std::map<int32_t, T> m)
{
    for (auto const& it : m) {
        if (!it.second.empty())
        {
            return it.first;
        }    
    }

    return -1;
}

eprosima::uxr::CustomAgent::InitFunction agent_custom_transport_open = [&]() -> bool
{
    return true;
};

eprosima::uxr::CustomAgent::FiniFunction agent_custom_transport_close = [&]() -> bool
{
    return true;
};

eprosima::uxr::CustomAgent::RecvMsgFunction agent_custom_transport_read_packet = [&](
            eprosima::uxr::CustomEndPoint* source_endpoint,
            uint8_t* buffer,
            size_t buffer_length,
            int timeout,
            eprosima::uxr::TransportRc& transport_rc) -> ssize_t
{
    size_t rv = 0;
    int64_t init_time = uxr_millis();
    bool received = false;

    transport_rc = eprosima::uxr::TransportRc::ok;

    while (uxr_millis() - init_time < timeout)
    {   
        int32_t index = find_queue_with_data(client_to_agent_packet_queue);
        if (0 <= index)
        {
            std::unique_lock<std::mutex> lock(client_to_agent_mtx);

            auto data = client_to_agent_packet_queue[index].front();
            client_to_agent_packet_queue[index].pop();

            if (data.size() <= buffer_length)
            {
                std::copy(data.begin(), data.end(), buffer);
                rv = data.size();
                received = true;
                printf("Custom agent receive: %ld in queue %d\n", rv, index);
                source_endpoint->set_member_value<uint32_t>("index", static_cast<uint32_t>(index));
            }
            else
            {
                transport_rc = eprosima::uxr::TransportRc::server_error;
            }
            
            break;
        }
    }
    
    if (!received)
    {
        transport_rc = eprosima::uxr::TransportRc::timeout_error;
    }
    
    return static_cast<ssize_t>(rv);
};

eprosima::uxr::CustomAgent::SendMsgFunction agent_custom_transport_write_packet = [&](
        const eprosima::uxr::CustomEndPoint* destination_endpoint,
        uint8_t* buffer,
        size_t message_length,
        eprosima::uxr::TransportRc& transport_rc) -> ssize_t
{
    std::unique_lock<std::mutex> lock(agent_to_client_mtx);
    int32_t index = static_cast<int32_t>(destination_endpoint->get_member<uint32_t>("index"));

    std::vector<uint8_t> packet(buffer, buffer + message_length);
    agent_to_client_packet_queue[index].emplace(std::move(packet));
    transport_rc = eprosima::uxr::TransportRc::ok;
    printf("Custom agent send: %ld\n", message_length);

    return static_cast<ssize_t>(message_length);
};

eprosima::uxr::CustomAgent::RecvMsgFunction agent_custom_transport_read_stream = [&](
            eprosima::uxr::CustomEndPoint* source_endpoint,
            uint8_t* buffer,
            size_t buffer_length,
            int timeout,
            eprosima::uxr::TransportRc& transport_rc) -> ssize_t
{
    size_t rv = 0;
    int64_t init_time = uxr_millis();
    bool received = false;

    transport_rc = eprosima::uxr::TransportRc::ok;

    while (uxr_millis() - init_time < timeout)
    {   
        int32_t index = find_queue_with_data(client_to_agent_stream_queue);
        if (0 <= index)
        {
            std::unique_lock<std::mutex> lock(client_to_agent_mtx);

            rv = (buffer_length > client_to_agent_stream_queue[index].size()) ?
                               client_to_agent_stream_queue[index].size() :
                               buffer_length;

            for (size_t i = 0; i < rv; i++)
            {
                buffer[i] = client_to_agent_stream_queue[index].front();
                client_to_agent_stream_queue[index].pop();
            }
            
            printf("Custom agent receive: %ld B in queue %d\n", rv, index);
            
            source_endpoint->set_member_value<uint32_t>("index", static_cast<uint32_t>(index));
            received = true;
            
            break;
        }
    }
    
    if (!received)
    {
        transport_rc = eprosima::uxr::TransportRc::timeout_error;
    }
    
    return static_cast<ssize_t>(rv);
};

eprosima::uxr::CustomAgent::SendMsgFunction agent_custom_transport_write_stream = [&](
        const eprosima::uxr::CustomEndPoint* destination_endpoint,
        uint8_t* buffer,
        size_t message_length,
        eprosima::uxr::TransportRc& transport_rc) -> ssize_t
{
    std::unique_lock<std::mutex> lock(agent_to_client_mtx);
    int32_t index = static_cast<int32_t>(destination_endpoint->get_member<uint32_t>("index"));

    for (size_t i = 0; i < message_length; i++)
    {
        agent_to_client_stream_queue[index].emplace(buffer[i]);
    }
    
    transport_rc = eprosima::uxr::TransportRc::ok;
    printf("Custom agent send: %ld to queue %d\n", message_length, index);

    return static_cast<ssize_t>(message_length);
};


// Client custom transport
extern "C"{
    static int32_t global_index = 0;

    bool client_custom_transport_open(uxrCustomTransport* transport)
    {
        transport->args = malloc(sizeof(int32_t));
        *(int32_t*) transport->args = global_index++;
        int32_t index = *(int32_t*) transport->args;

        printf("Custom client creating: %d\n", index);

        return true;
    }

    bool client_custom_transport_close(uxrCustomTransport* transport)
    {
        int32_t index = *(int32_t*) transport->args;
        free(transport->args);
        client_to_agent_packet_queue.erase(index);
        agent_to_client_packet_queue.erase(index);
        return true;
    }

    size_t client_custom_transport_write_packet(uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* errcode)
    {
        (void) errcode;

        int32_t index = *(int32_t*) transport->args;

        std::unique_lock<std::mutex> lock(client_to_agent_mtx);

        std::vector<uint8_t> packet(buf, buf + len);
        client_to_agent_packet_queue[index].emplace(std::move(packet));
        printf("Custom client send: %ld to queue %d\n", len, index);

        return len;
    }
    size_t client_custom_transport_read_packet(uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* errcode)
    {
        (void) errcode;

        int32_t index = *(int32_t*) transport->args;

        size_t rv = 0;
        int64_t init_time = uxr_millis();

        while (uxr_millis() - init_time < timeout)
        {
            if (0 < agent_to_client_packet_queue[index].size())
            {
                std::unique_lock<std::mutex> lock(agent_to_client_mtx);

                auto data = agent_to_client_packet_queue[index].front();
                agent_to_client_packet_queue[index].pop();

                if (data.size() <= len)
                {
                    std::copy( data.begin(), data.end(), buf);
                    rv = data.size();
                    printf("Custom client received: %ld from queue %d\n", rv, index);
                }
                else
                {
                    *errcode = 1;
                }
                
                break;
            }
        }
        
        return rv;
    }

    size_t client_custom_transport_write_stream(uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* errcode)
    {
        (void) errcode;

        int32_t index = *(int32_t*) transport->args;

        std::unique_lock<std::mutex> lock(client_to_agent_mtx);

        for (size_t i = 0; i < len; i++)
        {
            client_to_agent_stream_queue[index].emplace(buf[i]);
        }

        printf("Custom client send: %ld to queue %d\n", len, index);

        return len;
    }
    size_t client_custom_transport_read_stream(uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* errcode)
    {
        (void) errcode;

        int32_t index = *(int32_t*) transport->args;

        size_t rv = 0;
        int64_t init_time = uxr_millis();

        while (uxr_millis() - init_time < timeout)
        {
            if (0 < agent_to_client_stream_queue[index].size())
            {
                std::unique_lock<std::mutex> lock(agent_to_client_mtx);

                rv = (len > agent_to_client_stream_queue[index].size()) ?
                      agent_to_client_stream_queue[index].size() :
                      len;

                for (size_t i = 0; i < rv; i++)
                {
                    buf[i] = agent_to_client_stream_queue[index].front();
                    agent_to_client_stream_queue[index].pop();
                }
                
                break;
            }
        }
        
        return rv;
    }
}

