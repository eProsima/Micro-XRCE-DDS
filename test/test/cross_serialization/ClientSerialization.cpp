#include "ClientSerialization.hpp"

#include <core/serialization/xrce_protocol_internal.h>
#include <ucdr/microcdr.h>
#include <cstring>

#define BUFFER_LENGTH 1024

std::vector<uint8_t> ClientSerialization::create_client_payload()
{
    //change in a future by client_payload_sizeof function and remove resize
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    CREATE_CLIENT_Payload payload;
    payload.client_representation.xrce_cookie = XrceCookie{0x89, 0xAB, 0xCD, 0xEF};
    payload.client_representation.xrce_version = XrceVersion{0x01, 0x23};
    payload.client_representation.xrce_vendor_id = XrceVendorId{0x45, 0x67};
    payload.client_representation.client_key = ClientKey{0x89, 0xAB, 0xCD, 0xEF};
    payload.client_representation.session_id = 0x01;
    payload.client_representation.optional_properties = 0x00;
    payload.client_representation.mtu = 0x2345;
    uxr_serialize_CREATE_CLIENT_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::create_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    CREATE_Payload payload;
    payload.base.request_id = RequestId{0x01, 0x23};
    payload.base.object_id = ObjectId{0x45, 0x67};
    payload.object_representation.kind = OBJK_PARTICIPANT;
    payload.object_representation._.participant.base.representation.format = REPRESENTATION_BY_REFERENCE;
    payload.object_representation._.participant.domain_id = int16_t(0x09AB);
    payload.object_representation._.participant.base.representation._.object_reference = const_cast<char*>("ABCDE");

    uxr_serialize_CREATE_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::get_info_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    GET_INFO_Payload payload;
    payload.base.request_id = RequestId{0x01, 0x23};
    payload.base.object_id = ObjectId{0x45, 0x67};
    payload.info_mask = 0x89ABCDEF;
    uxr_serialize_GET_INFO_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::delete_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    DELETE_Payload payload;
    payload.base.request_id = RequestId{0x01, 0x23};
    payload.base.object_id = ObjectId{0x45, 0x67};
    uxr_serialize_DELETE_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::status_agent_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    STATUS_AGENT_Payload payload;
    payload.result.status = 0x01;
    payload.result.implementation_status = 0x23;
    payload.agent_info.xrce_cookie = XrceCookie{0x89, 0xAB, 0xCD, 0xEF};
    payload.agent_info.xrce_version = XrceVersion{0x01, 0x23};
    payload.agent_info.xrce_vendor_id = XrceVendorId{0x45, 0x67};
    payload.agent_info.optional_properties = 0x00;

    uxr_serialize_STATUS_AGENT_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::status_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    STATUS_Payload payload;
    payload.base.related_request.request_id = RequestId{0x01, 0x23};
    payload.base.related_request.object_id = ObjectId{0x45, 0x67};
    payload.base.result.implementation_status = 0x89;
    payload.base.result.status = 0xAB;
    uxr_serialize_STATUS_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::info_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    INFO_Payload payload;
    payload.base.related_request.request_id = RequestId{0x01, 0x23};
    payload.base.related_request.object_id = ObjectId{0x45, 0x67};
    payload.base.result.implementation_status = 0x89;
    payload.base.result.status = 0xAB;
    payload.object_info.optional_activity = 0x01;
    payload.object_info.optional_config = 0x01;
    payload.object_info.config.kind = OBJK_AGENT;
    payload.object_info.config._.agent.optional_properties = 0x00;
    payload.object_info.config._.agent.xrce_cookie = XrceCookie{0x89, 0xAB, 0xCD, 0xEF};
    payload.object_info.config._.agent.xrce_version = XrceVersion{0x01, 0x23};
    payload.object_info.config._.agent.xrce_vendor_id = XrceVendorId{0x45, 0x67};
    payload.object_info.activity.kind = OBJK_AGENT;
    payload.object_info.activity._.agent.availibility = 1;
    payload.object_info.activity._.agent.address_seq.size = 0x01;
    payload.object_info.activity._.agent.address_seq.data[0].format = ADDRESS_FORMAT_MEDIUM;
    payload.object_info.activity._.agent.address_seq.data[0]._.medium_locator.locator_port = 0x0123;
    payload.object_info.activity._.agent.address_seq.data[0]._.medium_locator.address[0] = 0x01;
    payload.object_info.activity._.agent.address_seq.data[0]._.medium_locator.address[1] = 0x23;
    payload.object_info.activity._.agent.address_seq.data[0]._.medium_locator.address[2] = 0x45;
    payload.object_info.activity._.agent.address_seq.data[0]._.medium_locator.address[3] = 0x67;

    uxr_serialize_INFO_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::read_data_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    READ_DATA_Payload payload;
    payload.base.request_id = RequestId{0x01, 0x23};
    payload.base.object_id = ObjectId{0x45, 0x67};
    payload.read_specification.preferred_stream_id = 0x80;
    payload.read_specification.data_format = 0x89;
    payload.read_specification.optional_content_filter_expression = 0x01;
    payload.read_specification.optional_delivery_control = 0x01;
    payload.read_specification.delivery_control.max_bytes_per_seconds = 0xABCD;
    payload.read_specification.delivery_control.max_elapsed_time = 0x2345;
    payload.read_specification.delivery_control.max_samples = 0xABCD;
    payload.read_specification.delivery_control.min_pace_period = 0xEF01;
    payload.read_specification.content_filter_expression = const_cast<char*>("ABCDE");

    uxr_serialize_READ_DATA_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::write_data_payload_data()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    WRITE_DATA_Payload_Data payload;
    payload.base.request_id = RequestId{0x01, 0x23};
    payload.base.object_id = ObjectId{0x45, 0x67};

    uxr_serialize_WRITE_DATA_Payload_Data(&ub, &payload);
    ucdr_serialize_array_char(&ub, "BYTES", 5);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::write_data_payload_sample()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::write_data_payload_data_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::write_data_payload_sample_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::write_data_payload_packed_samples()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::data_payload_data()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    BaseObjectRequest base;
    base.request_id = RequestId{0x01, 0x23};
    base.object_id = ObjectId{0x45, 0x67};

    uxr_serialize_BaseObjectRequest(&ub, &base);
    ucdr_serialize_array_char(&ub, "BYTES", 5);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::data_payload_sample()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::data_payload_data_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::data_payload_sample_seq()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::data_payload_packed_samples()
{
    //TODO
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientSerialization::acknack_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    ACKNACK_Payload payload;
    payload.first_unacked_seq_num = uint16_t(0x0123);
    payload.nack_bitmap[0] = uint8_t(0x45);
    payload.nack_bitmap[1] = uint8_t(0x67);
    payload.stream_id = uint8_t(0x89);
    uxr_serialize_ACKNACK_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

std::vector<uint8_t> ClientSerialization::heartbeat_payload()
{
    std::vector<uint8_t> buffer(BUFFER_LENGTH, 0x00);

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, &buffer.front(), uint32_t(buffer.capacity()));

    HEARTBEAT_Payload payload;
    payload.first_unacked_seq_nr = uint16_t(0x0123);
    payload.last_unacked_seq_nr = uint16_t(0x4567);
    payload.stream_id = uint8_t(0x89);
    uxr_serialize_HEARTBEAT_Payload(&ub, &payload);

    buffer.resize(std::size_t(ub.iterator - ub.init));

    return buffer;
}

