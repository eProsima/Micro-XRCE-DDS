#ifndef IN_TEST_GATEWAY_HPP
#define IN_TEST_GATEWAY_HPP

#include <iostream>
#include <random>
#include <chrono>
#include <uxr/client/core/communication/communication.h>

class Gateway
{
public:
    Gateway(float lost)
    : user_comm_(nullptr)
    , lost_(lost)
    {
        std::random_device rd;
        random_.seed(rd());
    }

    virtual ~Gateway()
    {
    }

    uxrCommunication* monitorize(uxrCommunication* user_comm)
    {
        user_comm_ = user_comm;
        communication_.instance = this;
        communication_.send_msg = send_dispatcher;
        communication_.recv_msg = recv_dispatcher;
        communication_.comm_error = user_comm->comm_error;
        communication_.mtu = user_comm->mtu;

        return &communication_;
    }

    float get_lost_value() const
    {
        return lost_;
    }

private:
    static const size_t MESSAGE_LENGTH = 4096;
    static std::uniform_real_distribution<float> msg_lost_;

    static bool send_dispatcher(void* instance, const uint8_t* buf, size_t len)
    {
        return static_cast<Gateway*>(instance)->send(buf, len);
    }

    static bool recv_dispatcher(void* instance, uint8_t** buf, size_t* len, int timeout)
    {
        return static_cast<Gateway*>(instance)->recv(buf, len, timeout);
    }

    bool send(const uint8_t* buf, size_t len)
    {
        if(get_lost())
        {
            std::cout << "[Message from client lost -> " << len << " bytes lost]" << std::endl;
            return false;
        }

        return user_comm_->send_msg(user_comm_->instance, buf, len);
    }

    bool recv(uint8_t** buf, size_t* len, int timeout)
    {
        int poll = timeout;
        do
        {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            bool result = user_comm_->recv_msg(user_comm_->instance, buf, len, timeout);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            poll -= static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());

            if(result)
            {
                if(get_lost())
                {
                    std::cout << "[Message from agent lost -> " << *len << " bytes lost]" << std::endl;
                }
                else
                {
                    return result;
                }
            }
        }
        while(0 < poll);

        return false;
    }

    bool get_lost()
    {
        if(msg_lost_(random_) < lost_)
        {
            return true;
        }

        return false;
    }

    std::mt19937 random_;

    uxrCommunication* user_comm_;
    uxrCommunication communication_;

    float lost_;
};

#endif //IN_TEST_GATEWAY
