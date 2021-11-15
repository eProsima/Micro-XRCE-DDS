#include <gtest/gtest.h>


#include <Client.hpp>

#include <thread>

#include "ClientAgentInteraction.hpp"


TEST_P(ClientAgentInteraction, InitCloseSession)
{
    for (int i = 0; i < ClientAgentInteraction::INIT_CLOSE_RETRIES; ++i)
    {
        TearDown();
        SetUp();
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationXMLBestEffort)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationXMLReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationBINBestEffort)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationBINReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationREFBestEffort)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x01, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, NewEntitiesCreationREFReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseXMLXMLReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseBINBINReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
    }
}

/* TODO (#3589): Fix XML and REF reference issue to enable this test.
TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseXMLREFReliable)
{
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml(1, 0x80, UXR_STATUS_OK, 0));
    ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
}
*/

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReuseREFREFReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_ref<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REUSE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceXMLXMLReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceBINBINReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, UXR_REPLACE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationNoReplaceXMLXMLReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationNoReplaceBINBINReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_ERR_ALREADY_EXISTS, 0));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceReuseXMLXMLReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::FASTRTPS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_xml<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, ExistantEntitiesCreationReplaceReuseBINBINReliable)
{
    switch (std::get<1>(GetParam()))
    {
        case MiddlewareKind::FASTDDS:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::FASTDDS>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
        case MiddlewareKind::FASTRTPS:
        {
            // Not implemented
            break;
        }
        case MiddlewareKind::CED:
        {
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK, 0));
            ASSERT_NO_FATAL_FAILURE(client_.create_entities_bin<MiddlewareKind::CED>(1, 0x80, UXR_STATUS_OK_MATCHED, UXR_REPLACE | UXR_REUSE));
            break;
        }
    }
}

TEST_P(ClientAgentInteraction, PingFromClientToAgent)
{
    const Transport transport_kind(std::get<0>(GetParam()));
    ASSERT_NO_FATAL_FAILURE(client_.ping_agent(transport_kind));
}

INSTANTIATE_TEST_CASE_P(
    Transports,
    ClientAgentInteraction,
    ::testing::Combine(
        ::testing::Values(Transport::UDP_IPV4_TRANSPORT, Transport::TCP_IPV4_TRANSPORT, Transport::UDP_IPV6_TRANSPORT, Transport::TCP_IPV6_TRANSPORT),
        ::testing::Values(MiddlewareKind::FASTDDS, MiddlewareKind::FASTRTPS, MiddlewareKind::CED)));

INSTANTIATE_TEST_CASE_P(
    CustomTransports,
    ClientAgentInteraction,
    ::testing::Combine(
        ::testing::Values(Transport::CUSTOM_WITHOUT_FRAMING, Transport::CUSTOM_WITH_FRAMING),
        ::testing::Values(MiddlewareKind::FASTDDS)));

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
