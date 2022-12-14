#include <gtest/gtest.h>
#include <Client.hpp>
#include <thread>

#include "ClientAgentSerial.hpp"

TEST_P(ClientAgentSerial, PingFromClientToAgent)
{
    const Transport transport_kind(std::get<0>(GetParam()));

    switch (transport_kind)
    {
        case Transport::SERIAL_TRANSPORT:
            ASSERT_NO_FATAL_FAILURE(client_serial_.ping_agent(transport_kind));
            break;

        case Transport::MULTISERIAL_TRANSPORT:
        {
            std::vector<std::thread> ping_thr;
            for (auto & element : clients_multiserial_)
            {
                std::thread ping_thread(&ClientSerial::ping_agent, &element, transport_kind);
                ping_thr.push_back(std::move(ping_thread));
            }

            for (auto & thr : ping_thr)
            {
                if (thr.joinable())
                {
                    thr.join();
                }
            }

            break;
        }

        default:
            break;
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    SerialTransports,
    ClientAgentSerial,
    ::testing::Combine(
        ::testing::Values(Transport::SERIAL_TRANSPORT, Transport::MULTISERIAL_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS)));