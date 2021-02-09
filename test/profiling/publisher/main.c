// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <uxr/client/client.h>
#include <ucdr/microcdr.h>

#include <stdio.h> //printf
#include <string.h> //strcmp
#include <stdlib.h> //atoi

#define STREAM_HISTORY  2
#define BUFFER_SIZE     100 * STREAM_HISTORY

int main(int args, char** argv)
{
    // CLI
    if(4 > args)
    {
        printf("usage: client_key  topic_name  topic_size \n");
        return 0;
    }

    uint32_t client_key = (uint32_t)atoi(argv[1]);
    char* topic_name = argv[2];
    uint16_t topic_size = (uint16_t)atoi(argv[3]);
    topic_size = (4 > topic_size) ? 4 : topic_size;
    uint32_t max_topics = 32u;

    // Transport
    uxrUDPTransport transport;
    if(!uxr_init_udp_transport(&transport, UXR_IPv4, "127.0.0.1", "2020"))
    {
        printf("Error at create transport.\n");
        return 1;
    }

    // Session
    uxrSession session;
    uxr_init_session(&session, &transport.comm, client_key);
    if(!uxr_create_session(&session))
    {
        printf("Error at create session.\n");
        return 1;
    }

    // Streams
    uint8_t output_besteffort_stream_buffer[UXR_CONFIG_UDP_TRANSPORT_MTU];
    uxrStreamId besteffort_out = uxr_create_output_best_effort_stream(&session, output_besteffort_stream_buffer, UXR_CONFIG_UDP_TRANSPORT_MTU);

    uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
    uxrStreamId reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
    uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    // Create entities
    uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    const char* participant_ref = "participant_name";
    uint16_t participant_req = uxr_buffer_create_participant_ref(&session, reliable_out, participant_id, 0, participant_ref, UXR_REPLACE);

    uxrObjectId topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
    const char* topic_ref = topic_name;
    uint16_t topic_req = uxr_buffer_create_topic_ref(&session, reliable_out, topic_id, participant_id, topic_ref, UXR_REPLACE);

    uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
    const char* publisher_xml = "";
    uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisher_id, participant_id, publisher_xml, UXR_REPLACE);

    uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
    const char* datawriter_ref = topic_ref;
    uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, datawriter_id, publisher_id, datawriter_ref, UXR_REPLACE);

    // Send create entities message and wait its status
    uint8_t status[4];
    uint16_t requests[4] = {participant_req, topic_req, publisher_req, datawriter_req};
    if(!uxr_run_session_until_all_status(&session, 1000, requests, status, 4))
    {
        printf("Error at create entities: participant: %i topic: %i publisher: %i darawriter: %i\n", status[0], status[1], status[2], status[3]);
        return 1;
    }

    // Write topics
    bool connected = true;
    uint32_t count = 0;
    while(connected && count < max_topics)
    {
        char topic[300] = {};
        memset(topic, 'A', topic_size);

        ucdrBuffer ub;
        uxr_prepare_output_stream(&session, besteffort_out, datawriter_id, &ub, 4 + strlen(topic));
        ucdr_serialize_string(&ub, topic);

        printf("Send topic %s, by %d\n", topic, client_key);
        connected = uxr_run_session_time(&session, 50);
        ++count;
    }

    // Delete resources
    uxr_delete_session(&session);
    uxr_close_udp_transport(&transport);

    return 0;
}
