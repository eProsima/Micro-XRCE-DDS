
#ifndef IN_TEST_CLIENTSERIAL_HPP
#define IN_TEST_CLIENTSERIAL_HPP

#include "Client.hpp"

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <uxr/agent/transport/serial/baud_rate_table_linux.h>

class ClientSerial : public Client
{
public:
    ClientSerial(float lost, uint16_t history)
    : Client(lost, history)
    {
    }

    virtual ~ClientSerial()
    {}

    static termios init_termios(const char * baudrate_str)
    {
        struct termios attr = {};

        /* Setting CONTROL OPTIONS. */
        attr.c_cflag |= unsigned(CREAD);    // Enable read.
        attr.c_cflag |= unsigned(CLOCAL);   // Set local mode.
        attr.c_cflag &= unsigned(~PARENB);  // Disable parity.
        attr.c_cflag &= unsigned(~CSTOPB);  // Set one stop bit.
        attr.c_cflag &= unsigned(~CSIZE);   // Mask the character size bits.
        attr.c_cflag |= unsigned(CS8);      // Set 8 data bits.
        attr.c_cflag &= unsigned(~CRTSCTS); // Disable hardware flow control.

        /* Setting LOCAL OPTIONS. */
        attr.c_lflag &= unsigned(~ICANON);  // Set non-canonical input.
        attr.c_lflag &= unsigned(~ECHO);    // Disable echoing of input characters.
        attr.c_lflag &= unsigned(~ECHOE);   // Disable echoing the erase character.
        attr.c_lflag &= unsigned(~ISIG);    // Disable SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals.

        /* Setting INPUT OPTIONS. */
        attr.c_iflag &= unsigned(~IXON);    // Disable output software flow control.
        attr.c_iflag &= unsigned(~IXOFF);   // Disable input software flow control.
        attr.c_iflag &= unsigned(~INPCK);   // Disable parity check.
        attr.c_iflag &= unsigned(~ISTRIP);  // Disable strip parity bits.
        attr.c_iflag &= unsigned(~IGNBRK);  // No ignore break condition.
        attr.c_iflag &= unsigned(~IGNCR);   // No ignore carrier return.
        attr.c_iflag &= unsigned(~INLCR);   // No map NL to CR.
        attr.c_iflag &= unsigned(~ICRNL);   // No map CR to NL.

        /* Setting OUTPUT OPTIONS. */
        attr.c_oflag &= unsigned(~OPOST);   // Set raw output.

        /* Setting OUTPUT CHARACTERS. */
        attr.c_cc[VMIN] = 10;
        attr.c_cc[VTIME] = 1;

        /* Setting baudrate. */
        speed_t baudrate = getBaudRate(baudrate_str);
        attr.c_ispeed = baudrate;
        attr.c_ospeed = baudrate;

        return attr;
    }

    void init_transport(Transport transport, const char* ip, const char* port)
    {
        (void) transport;
        (void) port;
        mtu_ = UXR_CONFIG_CUSTOM_TRANSPORT_MTU;
        int fd_ = open(ip, O_RDWR | O_NONBLOCK);
        ASSERT_TRUE(uxr_init_serial_transport(&serial_transport_, fd_, 0x00, 0x00));
        uxr_init_session(&session_, gateway_.monitorize(&serial_transport_.comm), client_key_);

        uxr_set_topic_callback(&session_, on_topic_multi_dispatcher, this);
        init_common();
    }

    void close_transport(Transport transport)
    {
        (void) transport;

        // Flash incomming messages.
        uxr_run_session_time(&session_, 100);

        bool deleted = uxr_delete_session(&session_);

        if(0.0f == gateway_.get_lost_value()) //because the agent only send one status to a delete in stream 0.
        {
            EXPECT_TRUE(deleted);
            EXPECT_EQ(UXR_STATUS_OK, session_.info.last_requested_status);
        }

        ASSERT_TRUE(uxr_close_serial_transport(&serial_transport_));
    }

    void ping_agent(
            const Transport transport_kind)
    {
        (void) transport_kind;
        uxrCommunication* comm = &serial_transport_.comm;
        ASSERT_TRUE(uxr_ping_agent_attempts(comm, 1000, 1));
    }

private:
    uxrSerialTransport serial_transport_;
};

#endif //IN_TEST_CLIENTSERIAL_HPP
