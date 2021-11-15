#include <gtest/gtest.h>
#include <Client.hpp>
#include <thread>

#include "ClientAgentCan.hpp"

TEST_P(ClientAgentCan, PingFromClientToAgent)
{
    ASSERT_NO_FATAL_FAILURE(client_can_.ping_agent());
}

INSTANTIATE_TEST_CASE_P(
    CanTransports,
    ClientAgentCan,
    ::testing::Values(MiddlewareKind::FASTDDS));