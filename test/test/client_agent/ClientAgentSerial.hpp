#ifndef IN_TEST_CLIENTSERIAL_INT_HPP
#define IN_TEST_CLIENTSERIAL_INT_HPP

#include <uxr/agent/transport/serial/MultiTermiosAgentLinux.hpp>
#include <uxr/agent/transport/serial/TermiosAgentLinux.hpp>
#include <ClientSerial.hpp>
#include <fcntl.h>

class AgentSerial
{
public:
    const char * baudrate = "115200";
    const char * port_name = "/dev/ptmx";
    const int client_number = 5;

    AgentSerial(Transport transport,
          MiddlewareKind middleware)
        : transport_(transport)
        , middleware_{}
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

    ~AgentSerial()
    {}

    void start()
    {
        switch(transport_)
        {
            case Transport::SERIAL_TRANSPORT:
            {
                struct termios attr = ClientSerial::init_termios(baudrate);
                agent_serial_.reset(new eprosima::uxr::TermiosAgent(port_name,  O_RDWR | O_NOCTTY, attr, 0, middleware_));
                agent_serial_->set_verbose_level(6);
                ASSERT_TRUE(agent_serial_->start());
                break;
            }
            case Transport::MULTISERIAL_TRANSPORT:
            {
                struct termios attr = ClientSerial::init_termios(baudrate);
                
                std::vector<std::string> devs;
                for (size_t i = 0; i < client_number; i++)
                {
                    devs.push_back(port_name);
                }
    
                agent_multiserial_.reset(new eprosima::uxr::MultiTermiosAgent(devs,  O_RDWR | O_NOCTTY, attr, 0, middleware_));
                ASSERT_TRUE(agent_multiserial_->start());
                break;
            }
        }
    }

    void stop()
    {
        switch(transport_)
        {
            case Transport::SERIAL_TRANSPORT:
            {
                ASSERT_TRUE(agent_serial_->stop());
                break;
            }
            case Transport::MULTISERIAL_TRANSPORT:
            {
                ASSERT_TRUE(agent_multiserial_->stop());
                break;
            }
        }
    }

    int getfd()
    {
        return agent_serial_->getfd();
    }

    std::vector<int> getfd_multi()
    {
        return agent_multiserial_->getfds();
    }

    bool wait_multiserial_open()
    {
        while (getfd_multi().size() != client_number)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        return true;
    }

private:
    Transport transport_;
    std::unique_ptr<eprosima::uxr::TermiosAgent> agent_serial_;
    std::unique_ptr<eprosima::uxr::MultiTermiosAgent> agent_multiserial_;

    eprosima::uxr::Middleware::Kind middleware_;
};

class ClientAgentSerial : public ::testing::TestWithParam<std::tuple<Transport, MiddlewareKind>>
{
public:

    ClientAgentSerial()
        : client_serial_(0.0f, 8)
        , transport_(std::get<0>(GetParam()))
        , clients_multiserial_{}
        , agent_(transport_, (MiddlewareKind) std::get<1>(GetParam()))
    {        
        for (size_t i = 0; i < agent_.client_number; i++)
        {
            ClientSerial auxcli(0.0f, 8);
            clients_multiserial_.push_back(auxcli);
        }
    }

    ~ClientAgentSerial()
    {}

    void SetUp() override
    {
        agent_.start();      

        switch (transport_)
        {
            case Transport::SERIAL_TRANSPORT:
            {
                int masterfd = agent_.getfd();
                grantpt(masterfd);
                unlockpt(masterfd);   
                ASSERT_NO_FATAL_FAILURE(client_serial_.init_transport(transport_, ptsname(masterfd), NULL));
                break;
            }
            case Transport::MULTISERIAL_TRANSPORT:
            {
                agent_.wait_multiserial_open();
                std::vector<int> masterfd = agent_.getfd_multi();

                for (size_t i = 0; i < masterfd.size(); i++)
                {
                    grantpt(masterfd[i]);
                    unlockpt(masterfd[i]);   
                    ASSERT_NO_FATAL_FAILURE(clients_multiserial_[i].init_transport(transport_, ptsname(masterfd[i]), NULL));
                }
                
                break;
            }
        }
    }

    void TearDown() override
    {
        switch (transport_)
        {
            case Transport::SERIAL_TRANSPORT:
            {
                ASSERT_NO_FATAL_FAILURE(client_serial_.close_transport(transport_));
                break;
            }
            case Transport::MULTISERIAL_TRANSPORT:
            {
                for (size_t i = 0; i < clients_multiserial_.size(); i++)
                {
                    ASSERT_NO_FATAL_FAILURE(clients_multiserial_[i].close_transport(transport_));
                }
                
            }
        }
        
        agent_.stop();
    }

protected:
    Transport transport_;
    AgentSerial agent_;
    
    std::vector<ClientSerial> clients_multiserial_;
    ClientSerial client_serial_;
};

#endif // ifndef IN_TEST_CLIENTSERIAL_INT_HPP