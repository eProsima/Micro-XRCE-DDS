#include <gtest/gtest.h>
#include <thread>

#include <Client.hpp>
#include "../client_agent/ClientAgentInteraction.hpp"

class ClientAgentInteractionThread : public ClientAgentInteraction
{
};

TEST_P(ClientAgentInteractionThread, PingFromClientToAgentThread)
{
    const Transport transport_kind(std::get<0>(GetParam()));

    std::string message("Hello DDS world!");
    std::thread ping_thread(&Client::ping_agent, client_, transport_kind, 10, 1);
    std::thread publisher_thread(&Client::publish_run, client_, 1, 1, 10, message);
    publisher_thread.join();
    ping_thread.join();
}

INSTANTIATE_TEST_CASE_P(
    Transports,
    ClientAgentInteractionThread,
    ::testing::Combine(
        ::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV6_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS, MiddlewareKind::FASTRTPS, MiddlewareKind::CED)));

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
