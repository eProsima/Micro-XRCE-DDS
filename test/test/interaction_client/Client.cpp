#include "Client.hpp"

uint32_t Client::next_client_key_ = 0;

bool flush_session(uxrSession* session){
    return uxr_run_session_until_confirm_delivery(session, 1000);
}