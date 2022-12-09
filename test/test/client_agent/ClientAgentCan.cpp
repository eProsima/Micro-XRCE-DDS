#include <gtest/gtest.h>
#include <Client.hpp>
#include <thread>

#include "ClientAgentCan.hpp"

TEST_P(ClientAgentCan, PingFromClientToAgent)
{
    ASSERT_NO_FATAL_FAILURE(client_can_.ping_agent());
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    CanTransports,
    ClientAgentCan,
    ::testing::Values(MiddlewareKind::FASTDDS));