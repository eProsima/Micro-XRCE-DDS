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

#include <shapesdemo/config.hpp>

#include <uxr/agent/Agent.hpp>
#if defined(PLATFORM_NAME_LINUX)
#include <uxr/agent/transport/udp/UDPServerLinux.hpp>
#include <uxr/agent/transport/tcp/TCPServerLinux.hpp>
#elif defined(PLATFORM_NAME_WINDOWS)
#include <uxr/agent/transport/udp/UDPServerWindows.hpp>
#include <uxr/agent/transport/tcp/TCPServerWindows.hpp>
#endif

#include <gtest/gtest.h>
#include <cstdlib>

#define UDP_TRANSPORT 1
#define TCP_TRANSPORT 2

class ShapesDemoTest : public ::testing::TestWithParam<int>
{
public:
    const uint16_t AGENT_PORT = 2018;
    ShapesDemoTest()
        : transport_(GetParam())
        , agent_(init_agent(AGENT_PORT))
    {
        agent_->run();
        agent_->load_config_file(UTEST_SHAPESDEMO_REFS);
    }

    void TearDown() override
    {
        std::string echo = "echo '";
        std::string executable = UTEST_SHAPESDEMO_COMMAND;
        std::string args = ((UDP_TRANSPORT == transport_) ? "--udp" : "--tcp") + std::string(" 127.0.0.1 ") + std::to_string(AGENT_PORT);

        std::string commands = "";
        for(std::vector<std::string>::iterator it = commands_.begin() ; it != commands_.end(); ++it)
        {
            commands.append(*it + " \n ");
        }

        std::string execution = echo + commands + executable + args;
        int shape_demo_app_result = std::system(execution.c_str());

        ASSERT_EQ(0, shape_demo_app_result);
    }

protected:
    eprosima::uxr::Server* init_agent(uint16_t port) const
    {
        eprosima::uxr::Server* agent = nullptr;
        switch(transport_)
        {
            case UDP_TRANSPORT:
                agent = new eprosima::uxr::UDPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST);
                break;
            case TCP_TRANSPORT:
                agent = new eprosima::uxr::TCPv4Agent(port, eprosima::uxr::Middleware::Kind::FAST);
                break;
            default:
                std::cerr << "Not supported transport" << std::endl;
                break;
        }

        return agent;
    }

    int transport_;
    std::unique_ptr<eprosima::uxr::Server> agent_;
    std::vector<std::string> commands_;
};

/* ############################################## TESTS ##################################################### */

INSTANTIATE_TEST_CASE_P(Transport, ShapesDemoTest, ::testing::Values(UDP_TRANSPORT, TCP_TRANSPORT), ::testing::PrintToStringParamName());

TEST_P(ShapesDemoTest, CreateSession)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, CreateDeleteSession)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, CreateEntitiesTree)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("tree 1");
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, DeleteEntity)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("tree 1");
    commands_.emplace_back("delete 1 1"); //Removed the participant, all associated elements will be removed
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, PublishTopicBestEffort)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("tree 1");
    commands_.emplace_back("write_data 1 1 50 50 100 BLUE");
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, PublishTopicReliable)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("tree 1");
    commands_.emplace_back("write_data 1 128 60 70 80 GREEN");
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, PublishSubscribeTopicBestEffort)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("tree 1");
    commands_.emplace_back("request_data 1 1 1");
    commands_.emplace_back("write_data 1 1 60 70 80 GREEN");
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, PublishSubscribeTopicReliable)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("tree 1");
    commands_.emplace_back("request_data 1 80 1");
    commands_.emplace_back("write_data 1 128 60 70 80 GREEN");
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}

TEST_P(ShapesDemoTest, Complete)
{
    commands_.emplace_back("create_session");
    commands_.emplace_back("create_participant 1");
    commands_.emplace_back("create_topic 1 1");
    commands_.emplace_back("create_publisher 1 1");
    commands_.emplace_back("create_datawriter 1 1");
    commands_.emplace_back("create_subscriber 1 1");
    commands_.emplace_back("create_datareader 1 1");
    commands_.emplace_back("tree 2");
    commands_.emplace_back("request_data 1 1 2");
    commands_.emplace_back("write_data 1 1 50 0 80 GREEN");
    commands_.emplace_back("write_data 1 1 0 50 80 GREEN");
    commands_.emplace_back("request_data 1 128 2");
    commands_.emplace_back("write_data 1 128 80 0 80 BLUE");
    commands_.emplace_back("write_data 1 128 0 80 80 BLUE");
    commands_.emplace_back("delete_session");
    commands_.emplace_back("exit");
}
