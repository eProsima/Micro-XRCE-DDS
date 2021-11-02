#ifndef IN_TEST_CLIENTCAN_INT_HPP
#define IN_TEST_CLIENTCAN_INT_HPP

#include <uxr/agent/transport/can/CanAgentLinux.hpp>
#include <ClientCan.hpp>
#include <fcntl.h>
#include <stdlib.h>

static bool initialized = false;

class AgentCan
{
public:
    const char * dev = "vcan0";
    const uint32_t can_id = 0x00000001; // TODO: test different can_id

    AgentCan(MiddlewareKind middleware)
        : middleware_{}
    {
        switch (middleware)
        {
            case MiddlewareKind::FASTDDS:
                middleware_ = eprosima::uxr::Middleware::Kind::FASTDDS;
                break;
            case MiddlewareKind::FASTRTPS:
                middleware_ = eprosima::uxr::Middleware::Kind::FASTRTPS;
                break;
            case MiddlewareKind::CED:
                middleware_ = eprosima::uxr::Middleware::Kind::CED;
                break;
        }
    }

    ~AgentCan()
    {}

    void start()
    {
        if (!initialized)
        {
            ASSERT_TRUE(0 == system("sudo ip link add dev vcan0 type vcan && sudo ip link set vcan0 mtu 72 && sudo ip link set dev vcan0 up"));
            initialized = true;
        }

        agent_can_.reset(new eprosima::uxr::CanAgent(dev, can_id, middleware_));
        agent_can_->set_verbose_level(6);
        ASSERT_TRUE(agent_can_->start());
    }

    void stop()
    {
        ASSERT_TRUE(agent_can_->stop());
    }

private:
    std::unique_ptr<eprosima::uxr::CanAgent> agent_can_;
    eprosima::uxr::Middleware::Kind middleware_;
};

class ClientAgentCan : public ::testing::TestWithParam<MiddlewareKind>
{
public:
    ClientAgentCan()
        : client_can_(0.0f, 8)
        , agent_(GetParam())
    {}

    ~ClientAgentCan()
    {}

    void SetUp() override
    {
        agent_.start();
        ASSERT_NO_FATAL_FAILURE(client_can_.init_transport(agent_.dev, agent_.can_id + 1));
    }

    void TearDown() override
    {
        ASSERT_NO_FATAL_FAILURE(client_can_.close_transport());
        agent_.stop();
    }

protected:
    ClientCan client_can_;
    AgentCan agent_;
};

#endif // ifndef IN_TEST_CLIENTCAN_INT_HPP