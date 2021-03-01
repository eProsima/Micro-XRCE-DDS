#include "Custom_transports.hpp"
#include <uxr/client/util/time.h>

#include <chrono>
#include <thread>
#include <queue>

using packet_fifo = std::queue<std::vector<uint8_t>>;
using stream_fifo = std::queue<uint8_t>;

static std::map<int32_t, packet_fifo> client_to_agent_packet_queue;
static std::map<int32_t, packet_fifo> agent_to_client_packet_queue;
static std::map<int32_t, stream_fifo> client_to_agent_stream_queue;
static std::map<int32_t, stream_fifo> agent_to_client_stream_queue;

std::mutex transport_mtx;

template <class T>
static int32_t find_queue_with_data(const std::map<int32_t, T>& m)
{
    for (auto const& it : m)
    {
        if (!it.second.empty())
        {
            return it.first;
        }    
    }

    return -1;
}

template <class T>
static void erase_fifo_by_index(std::map<int32_t, T>& m, const int32_t index)
{
    auto it = m.find(index);
    if (it != m.end())
        m.erase (it);
}

eprosima::uxr::CustomAgent::InitFunction agent_custom_transport_open = []() -> bool
{
    return true;
};

eprosima::uxr::CustomAgent::FiniFunction agent_custom_transport_close = []() -> bool
{
    std::unique_lock<std::mutex> lock(transport_mtx);

    client_to_agent_stream_queue.clear();
    client_to_agent_packet_queue.clear();
    agent_to_client_stream_queue.clear();
    agent_to_client_stream_queue.clear();
    
    return true;
};

eprosima::uxr::CustomAgent::RecvMsgFunction agent_custom_transport_read_packet = [](
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
        std::unique_lock<std::mutex> lock(transport_mtx);

        int32_t index = find_queue_with_data(client_to_agent_packet_queue);
        if (0 <= index)
        {
            auto data = client_to_agent_packet_queue[index].front();
            client_to_agent_packet_queue[index].pop();

            if (data.size() <= buffer_length)
            {
                std::copy(data.begin(), data.end(), buffer);
                rv = data.size();
                received = true;
                std::cout << "Custom agent receive: " << rv << " bytes in queue " <<  index << std::endl;
                source_endpoint->set_member_value<uint32_t>("index", static_cast<uint32_t>(index));
            }
            else
            {
                transport_rc = eprosima::uxr::TransportRc::server_error;
            }
            
            break;
        }

        lock.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    
    if (!received)
    {
        transport_rc = eprosima::uxr::TransportRc::timeout_error;
    }
    
    return static_cast<ssize_t>(rv);
};

eprosima::uxr::CustomAgent::SendMsgFunction agent_custom_transport_write_packet = [](
        const eprosima::uxr::CustomEndPoint* destination_endpoint,
        uint8_t* buffer,
        size_t message_length,
        eprosima::uxr::TransportRc& transport_rc) -> ssize_t
{
    std::unique_lock<std::mutex> lock(transport_mtx);
    int32_t index = static_cast<int32_t>(destination_endpoint->get_member<uint32_t>("index"));

    std::vector<uint8_t> packet(buffer, buffer + message_length);
    agent_to_client_packet_queue[index].emplace(std::move(packet));
    transport_rc = eprosima::uxr::TransportRc::ok;
    std::cout << "Custom agent send: " << message_length << " bytes." << std::endl;

    return static_cast<ssize_t>(message_length);
};

eprosima::uxr::CustomAgent::RecvMsgFunction agent_custom_transport_read_stream = [](
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
        std::unique_lock<std::mutex> lock(transport_mtx);

        int32_t index = find_queue_with_data(client_to_agent_stream_queue);
        if (0 <= index)
        {
            rv = (buffer_length > client_to_agent_stream_queue[index].size()) ?
                               client_to_agent_stream_queue[index].size() :
                               buffer_length;

            for (size_t i = 0; i < rv; i++)
            {
                buffer[i] = client_to_agent_stream_queue[index].front();
                client_to_agent_stream_queue[index].pop();
            }
            
            std::cout << "Custom agent receive: " << rv << " bytes in queue " << index << std::endl;
            
            source_endpoint->set_member_value<uint32_t>("index", static_cast<uint32_t>(index));
            received = true;
            
            break;
        }
 
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    
    if (!received)
    {
        transport_rc = eprosima::uxr::TransportRc::timeout_error;
    }
    
    return static_cast<ssize_t>(rv);
};

eprosima::uxr::CustomAgent::SendMsgFunction agent_custom_transport_write_stream = [](
        const eprosima::uxr::CustomEndPoint* destination_endpoint,
        uint8_t* buffer,
        size_t message_length,
        eprosima::uxr::TransportRc& transport_rc) -> ssize_t
{
    std::unique_lock<std::mutex> lock(transport_mtx);
    int32_t index = static_cast<int32_t>(destination_endpoint->get_member<uint32_t>("index"));

    for (size_t i = 0; i < message_length; i++)
    {
        agent_to_client_stream_queue[index].emplace(buffer[i]);
    }
    
    transport_rc = eprosima::uxr::TransportRc::ok;
    std::cout << "Custom agent send: " << message_length << " bytes to queue " << index << std::endl;

    return static_cast<ssize_t>(message_length);
};


// Client custom transport
extern "C"
{
    static int32_t global_index = 0;

    bool client_custom_transport_open(uxrCustomTransport* transport)
    {
        transport->args = malloc(sizeof(int32_t));
        *(int32_t*) transport->args = global_index++;
        int32_t index = *(int32_t*) transport->args;

        std::cout << "Custom client creating: " << index << std::endl;

        return true;
    }

    bool client_custom_transport_close(uxrCustomTransport* transport)
    {
        int32_t index = *(int32_t*) transport->args;
        free(transport->args);
        
        std::unique_lock<std::mutex> lock(transport_mtx);

        erase_fifo_by_index(client_to_agent_packet_queue, index);
        erase_fifo_by_index(client_to_agent_stream_queue, index);
        erase_fifo_by_index(agent_to_client_packet_queue, index);
        erase_fifo_by_index(agent_to_client_stream_queue, index);

        return true;
    }

    size_t client_custom_transport_write_packet(uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* errcode)
    {
        (void) errcode;

        int32_t index = *(int32_t*) transport->args;

        std::unique_lock<std::mutex> lock(transport_mtx);

        std::vector<uint8_t> packet(buf, buf + len);
        client_to_agent_packet_queue[index].emplace(std::move(packet));
        std::cout << "Custom client send: " << len << " bytes in queue " << index << std::endl;

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
            std::unique_lock<std::mutex> lock(transport_mtx);

            if (0 < agent_to_client_packet_queue[index].size())
            {
                auto data = agent_to_client_packet_queue[index].front();
                agent_to_client_packet_queue[index].pop();

                if (data.size() <= len)
                {
                    std::copy( data.begin(), data.end(), buf);
                    rv = data.size();
                    std::cout << "Custom client receive: " << len << " bytes in queue " << index << std::endl;
                }
                else
                {
                    *errcode = 1;
                }
                
                break;
            }
            
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        
        return rv;
    }

    size_t client_custom_transport_write_stream(uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* errcode)
    {
        (void) errcode;

        int32_t index = *(int32_t*) transport->args;

        std::unique_lock<std::mutex> lock(transport_mtx);

        for (size_t i = 0; i < len; i++)
        {
            client_to_agent_stream_queue[index].emplace(buf[i]);
        }

        std::cout << "Custom client send: " << len << " bytes in queue " << index << std::endl;

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
            std::unique_lock<std::mutex> lock(transport_mtx);

            if (0 < agent_to_client_stream_queue[index].size())
            {
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
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        
        return rv;
    }
}
