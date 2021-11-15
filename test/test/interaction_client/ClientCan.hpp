
#ifndef IN_TEST_CLIENTCAN_HPP
#define IN_TEST_CLIENTCAN_HPP

#include "Client.hpp"

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

class ClientCan : public Client
{
public:
    ClientCan(float lost, uint16_t history)
    : Client(lost, history)
    {
    }

    virtual ~ClientCan()
    {}

    void init_transport(const char* dev, const uint32_t can_id)
    {
        mtu_ = UXR_CONFIG_CAN_TRANSPORT_MTU;
        ASSERT_TRUE(uxr_init_can_transport(&can_transport_, dev, can_id));
        uxr_init_session(&session_, gateway_.monitorize(&can_transport_.comm), client_key_);
        init_common();
    }

    void close_transport()
    {
        // Flash incomming messages.
        uxr_run_session_time(&session_, 100);

        bool deleted = uxr_delete_session(&session_);

        if(0.0f == gateway_.get_lost_value()) //because the agent only send one status to a delete in stream 0.
        {
            EXPECT_TRUE(deleted);
            EXPECT_EQ(UXR_STATUS_OK, session_.info.last_requested_status);
        }

        ASSERT_TRUE(uxr_close_can_transport(&can_transport_));
    }

    void ping_agent()
    {
        uxrCommunication* comm = &can_transport_.comm;
        ASSERT_TRUE(uxr_ping_agent_attempts(comm, 1000, 1));
    }

private:
    uxrCANTransport can_transport_;
};

#endif //IN_TEST_CLIENTCAN_HPP
