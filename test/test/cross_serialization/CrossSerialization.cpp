/****************************************************************************
 *
 * Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <gtest/gtest.h>
#include "ClientSerialization.hpp"
#include "AgentSerialization.hpp"

#define AGENT_HEADER_OFFSET 12 //Used for skip the agent header and subheader

class CrossSerializationTests : public testing::Test
{
public:
    void TearDown() override
    {
        agent_ser.erase(agent_ser.begin(), agent_ser.begin() + AGENT_HEADER_OFFSET);
        EXPECT_EQ(client_ser, agent_ser);
    }

protected:
    std::vector<uint8_t> client_ser;
    std::vector<uint8_t> agent_ser;
};

/* ############################################## TESTS ##################################################### */

TEST_F(CrossSerializationTests, CreateClientPayload)
{
    client_ser = ClientSerialization::create_client_payload();
    agent_ser = AgentSerialization::create_client_payload();
}

TEST_F(CrossSerializationTests, CreatePayload)
{
    client_ser = ClientSerialization::create_payload();
    agent_ser = AgentSerialization::create_payload();
}

TEST_F(CrossSerializationTests, DeletePayload)
{
    client_ser = ClientSerialization::delete_payload();
    agent_ser = AgentSerialization::delete_payload();
}

TEST_F(CrossSerializationTests, StatusPayload)
{
    client_ser = ClientSerialization::status_payload();
    agent_ser = AgentSerialization::status_payload();
}

TEST_F(CrossSerializationTests, ReadDataPayload)
{
    client_ser = ClientSerialization::read_data_payload();
    agent_ser = AgentSerialization::read_data_payload();
}

TEST_F(CrossSerializationTests, WriteDataPayloadData)
{
    client_ser = ClientSerialization::write_data_payload_data();
    agent_ser = AgentSerialization::write_data_payload_data();
}

TEST_F(CrossSerializationTests, DataPayloadData)
{
    client_ser = ClientSerialization::data_payload_data();
    agent_ser = AgentSerialization::data_payload_data();
}

TEST_F(CrossSerializationTests, AcknackPayload)
{
    client_ser = ClientSerialization::acknack_payload();
    agent_ser = AgentSerialization::acknack_payload();
}

TEST_F(CrossSerializationTests, HeartbeatPayload)
{
    client_ser = ClientSerialization::heartbeat_payload();
    agent_ser = AgentSerialization::heartbeat_payload();
}

TEST_F(CrossSerializationTests, GetInfoPayload)
{
    client_ser = ClientSerialization::get_info_payload();
    agent_ser = AgentSerialization::get_info_payload();
}

TEST_F(CrossSerializationTests, InfoPayload)
{
    client_ser = ClientSerialization::info_payload();
    agent_ser = AgentSerialization::info_payload();
}
