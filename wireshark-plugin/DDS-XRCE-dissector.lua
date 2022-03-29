-- DDS-XRCE protocol dissector for Wireshark
--
-- Copyright 2022 Lely Industries N.V.
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

local p_dds_xrce = Proto("DDS-XRCE", "DDS for eXtremely Resource Constrained Environments")

local f_session_id = ProtoField.uint8("dds-xrce.session_id", "sessionId", base.HEX)
local f_stream_id = ProtoField.uint8("dds-xrce.stream_id", "streamId", base.HEX)
local f_sequence_nr = ProtoField.uint16("dds-xrce.sequence_nr", "sequenceNr", base.DEC)
local f_client_key = ProtoField.uint32("dds-xrce.client_key", "clientKey", base.HEX)

local f_submessage = ProtoField.none("dds-xrce.submessage", "submessage")
local f_submessage_id = ProtoField.uint8("dds-xrce.submessage.id", "submessageId", base.DEC)
local f_flags = ProtoField.uint8("dds-xrce.submessage.flags", "flags", base.HEX)
local f_flags_endianness =
    ProtoField.uint8(
    "dds-xrce.submessage.flags.endianness",
    "Endianness bit",
    base.HEX,
    {[0] = "Not set", [1] = "Set"},
    0x01
)
local f_flags_reuse =
    ProtoField.uint8("dds-xrce.submessage.flags.reuse", "Reuse bit", base.HEX, {[0] = "Not set", [1] = "Set"}, 0x02)
local f_flags_replace =
    ProtoField.uint8("dds-xrce.submessage.flags.replace", "Replace bit", base.HEX, {[0] = "Not set", [1] = "Set"}, 0x04)
local f_flags_data_format =
    ProtoField.uint8(
    "dds-xrce.submessage.flags.data_format",
    "DataFormat",
    base.HEX,
    {
        [0x0] = "FORMAT_DATA",
        [0x2] = "FORMAT_SAMPLE",
        [0x8] = "FORMAT_DATA_SEQ",
        [0xA] = "FORMAT_SAMPLE_SEQ",
        [0xE] = "FORMAT_PACKED_SAMPLES"
    },
    0x0E
)
local f_flags_last_fragment =
    ProtoField.uint8(
    "dds-xrce.submessage.flags.last_fragment",
    "Last Fragment bit",
    base.HEX,
    {[0] = "Not set", [1] = "Set"},
    0x04
)
local f_submessage_length = ProtoField.uint16("dds-xrce.submessage.length", "submessageLength", base.DEC)
local f_payload = ProtoField.bytes("dds-xrce.submessage.payload", "payload", base.SPACE)

p_dds_xrce.fields = {
    f_session_id,
    f_stream_id,
    f_sequence_nr,
    f_client_key,
    f_submessage,
    f_submessage_id,
    f_flags,
    f_flags_endianness,
    f_flags_reuse,
    f_flags_replace,
    f_flags_data_format,
    f_flags_last_fragment,
    f_submessage_length,
    f_payload
}

function p_dds_xrce.dissector(tvb, pinfo, tree)
    pinfo.cols.protocol = "DDS-XRCE"

    local t_dds_xrce = tree:add(p_dds_xrce, tvb(), "DDS-XRCE Protocol")
    local offset = 0

    local session_id = tvb(offset, 1):uint()
    local t_session_id = t_dds_xrce:add(f_session_id, tvb(offset, 1))
    if session_id == 0x00 then
        t_session_id:append_text(" (SESSION_ID_NONE_WITH_CLIENT_KEY)")
    elseif session_id == 0x80 then
        t_session_id:append_text(" (SESSION_ID_NONE_WITHOUT_CLIENT_KEY)")
    end
    offset = offset + 1

    local stream_id = tvb(offset, 1):uint()
    local t_stream_id = t_dds_xrce:add(f_stream_id, tvb(offset, 1))
    if stream_id == 0x00 then
        t_stream_id:append_text(" (STREAMID_NONE)")
    elseif stream_id == 0x01 then
        t_stream_id:append_text(" (STREAMID_BUILTIN_BEST_EFFORTS)")
    elseif stream_id == 0x80 then
        t_stream_id:append_text(" (STREAMID_BUILTIN_RELIABLE)")
    end
    if stream_id >= 0x80 then
        t_stream_id:add(tvb(offset, 1), "reliability:", "RELIABLE")
    elseif stream_id >= 0x01 then
        t_stream_id:add(tvb(offset, 1), "reliability:", "BEST_EFFORTS")
    end
    offset = offset + 1

    t_dds_xrce:add_le(f_sequence_nr, tvb(offset, 2))
    offset = offset + 2

    if session_id < 0x80 then
        t_dds_xrce:add_le(f_client_key, tvb(offset, 4))
        offset = offset + 4
    end

    while offset < tvb:len() do
        local t_submessage = t_dds_xrce:add(f_submessage, tvb(offset))

        local submessage_id = tvb(offset, 1):uint()
        t_submessage:add(f_submessage_id, tvb(offset, 1))
        offset = offset + 1

        local flags = tvb(offset, 1):uint()
        local t_flags = t_submessage:add(f_flags, tvb(offset, 1))
        if submessage_id == 1 then
            -- CREATE
            if bit.band(flags, 0x04) == 0x04 then
                t_flags:append_text(", Replace bit")
            end
            t_flags:add(f_flags_replace, tvb(offset, 1))
            if bit.band(flags, 0x02) == 0x02 then
                t_flags:append_text(", Reuse bit")
            end
            t_flags:add(f_flags_reuse, tvb(offset, 1))
        elseif submessage_id == 7 or submessage_id == 9 then
            -- WRITE_DATA or DATA
            if bit.band(flags, 0x0E) == 0x00 then
                t_flags:append_text(", FORMAT_DATA")
            elseif bit.band(flags, 0x0E) == 0x02 then
                t_flags:append_text(", FORMAT_SAMPLE")
            elseif bit.band(flags, 0x0E) == 0x08 then
                t_flags:append_text(", FORMAT_DATA_SEQ")
            elseif bit.band(flags, 0x0E) == 0x0A then
                t_flags:append_text(", FORMAT_SAMPLE_SEQ")
            elseif bit.band(flags, 0x0E) == 0x0E then
                t_flags:append_text(", FORMAT_PACKED_SAMPLES")
            end
            t_flags:add(f_flags_data_format, tvb(offset, 1))
        elseif submessage_id == 13 then
            -- FRAGMENT
            if bit.band(flags, 0x02) == 0x02 then
                t_flags:append_text(", Last Fragment bit")
            end
            t_flags:add(f_flags_last_fragment, tvb(offset, 1))
        end
        local encoding = ENC_BIG_ENDIAN
        if bit.band(flags, 0x01) == 0x01 then
            t_flags:append_text(", Endianness bit")
            encoding = ENC_LITTLE_ENDIAN
        end
        t_flags:add(f_flags_endianness, tvb(offset, 1))
        offset = offset + 1

        submessage_length = tvb(offset, 2):le_uint()
        t_submessage:add_le(f_submessage_length, tvb(offset, 2))
        offset = offset + 2

        t_submessage:set_len(4 + submessage_length)

        local t_payload = t_submessage:add(f_payload, tvb(offset, submessage_length))

        if submessage_id == 0 then
            t_submessage:set_text("CREATE_CLIENT")
            t_payload:set_text("CREATE_CLIENT_Payload")
            idl_create_client_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 1 then
            t_submessage:set_text("CREATE")
            t_payload:set_text("CREATE_Payload")
            idl_create_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 2 then
            t_submessage:set_text("GET_INFO")
            t_payload:set_text("GET_INFO_Payload")
            idl_get_info_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 3 then
            t_submessage:set_text("DELETE")
            t_payload:set_text("DELETE_Payload")
            idl_delete_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 4 then
            t_submessage:set_text("STATUS_AGENT")
            t_payload:set_text("STATUS_AGENT_Payload")
            idl_status_agent_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 5 then
            t_submessage:set_text("STATUS")
            t_payload:set_text("STATUS_Payload")
            idl_status_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 6 then
            t_submessage:set_text("INFO")
            t_payload:set_text("INFO_Payload")
            idl_info_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 7 then
            t_submessage:set_text("WRITE_DATA")
            if bit.band(flags, 0x0E) == 0x00 then
                -- FORMAT_DATA
                t_payload:set_text("WRITE_DATA_Payload_Data")
                idl_write_data_payload_data(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x02 then
                -- FORMAT_SAMPLE
                t_payload:set_text("WRITE_DATA_Payload_Sample")
                idl_write_data_payload_sample(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x08 then
                -- FORMAT_DATA_SEQ
                t_payload:set_text("WRITE_DATA_Payload_DataSeq")
                idl_write_data_payload_data_seq(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x0A then
                -- FORMAT_SAMPLE_SEQ
                t_payload:set_text("WRITE_DATA_Payload_SampleSeq")
                idl_write_data_payload_sample_seq(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x0E then
                -- FORMAT_PACKED_SAMPLES
                t_payload:set_text("WRITE_DATA_Payload_PackedSamples")
                idl_write_data_payload_packed_samples(t_payload, tvb(offset, submessage_length), 0, encoding)
            end
        elseif submessage_id == 8 then
            t_submessage:set_text("READ_DATA")
            t_payload:set_text("READ_DATA_Payload")
            idl_read_data_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 9 then
            t_submessage:set_text("DATA")
            if bit.band(flags, 0x0E) == 0x00 then
                -- FORMAT_DATA
                t_payload:set_text("DATA_Payload_Data")
                idl_data_payload_data(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x02 then
                -- FORMAT_SAMPLE
                t_payload:set_text("DATA_Payload_Sample")
                idl_data_payload_sample(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x08 then
                -- FORMAT_DATA_SEQ
                t_payload:set_text("DATA_Payload_DataSeq")
                idl_data_payload_data_seq(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x0A then
                -- FORMAT_SAMPLE_SEQ
                t_payload:set_text("DATA_Payload_SampleSeq")
                idl_data_payload_sample_seq(t_payload, tvb(offset, submessage_length), 0, encoding)
            elseif bit.band(flags, 0x0E) == 0x0E then
                -- FORMAT_PACKED_SAMPLES
                t_payload:set_text("DATA_Payload_PackedSamples")
                idl_data_payload_packed_samples(t_payload, tvb(offset, submessage_length), 0, encoding)
            end
        elseif submessage_id == 10 then
            t_submessage:set_text("ACKNACK")
            t_payload:set_text("ACKNACK_Payload")
            idl_acknack_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 11 then
            t_submessage:set_text("HEARTBEAT")
            t_payload:set_text("HEARTBEAT_Payload")
            idl_heartbeat_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 12 then
            t_submessage:set_text("RESET")
        elseif submessage_id == 13 then
            t_submessage:set_text("FRAGMENT")
        elseif submessage_id == 14 then
            t_submessage:set_text("TIMESTAMP")
            t_payload:set_text("TIMESTAMP_Payload")
            idl_timestamp_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        elseif submessage_id == 15 then
            t_submessage:set_text("TIMESTAMP_REPLY")
            t_payload:set_text("TIMESTAMP_REPLY_Payload")
            idl_timestamp_reply_payload(t_payload, tvb(offset, submessage_length), 0, encoding)
        end

        offset = offset + align(submessage_length, 4)
    end
end

local udp_table = DissectorTable.get("udp.port")
udp_table:add(2018, p_dds_xrce)

function align(x, a)
    return bit.band(x + a - 1, bit.bnot(a - 1))
end

function tvb_int(tvb, encoding)
    encoding = encoding or ENC_BIG_ENDIAN
    local value = 0
    if encoding == ENC_BIG_ENDIAN then
        if tvb:len() <= 4 then
            value = tvb:int()
        else
            value = tvb:int64()
        end
    elseif encoding == ENC_LITTLE_ENDIAN then
        if tvb:len() <= 4 then
            value = tvb:le_int()
        else
            value = tvb:le_int64()
        end
    end
    return value
end

function tvb_uint(tvb, encoding)
    encoding = encoding or ENC_BIG_ENDIAN
    local value = 0
    if encoding == ENC_BIG_ENDIAN then
        if tvb:len() <= 4 then
            value = tvb:uint()
        else
            value = tvb:uint64()
        end
    elseif encoding == ENC_LITTLE_ENDIAN then
        if tvb:len() <= 4 then
            value = tvb:le_uint()
        else
            value = tvb:le_uint64()
        end
    end
    return value
end

function tvb_format_uint(tvb, encoding, b)
    b = b or base.DEC

    value = tvb_uint(tvb, encoding)
    if b == base.DEC then
        return string.format("%u", value)
    elseif b == base.HEX then
        return string.format("0x%0" .. tostring(tvb:len() * 2) .. "x", value)
    end
end

function idl_client_key(tree, tvb, offset, encoding, label)
    label = label or "client_key:"

    tree:add(tvb(offset, 4), label, tvb(offset, 4):bytes():tohex(true))
    offset = offset + 4

    return offset
end

function idl_object_id(tree, tvb, offset, encoding, label)
    label = label or "object_id:"

    local object_id_0 = tvb(offset, 1):uint()
    local object_id_1 = tvb(offset + 1, 1):uint()
    local t_object_id = tree:add(tvb(offset, 2), label, tvb(offset, 2):bytes():tohex(true))
    if object_id_0 == 0x00 and object_id_1 == 0x00 then
        t_object_id:append_text(" (INVALID)")
    elseif object_id_0 == 0xff then
        if object_id_1 == 0xfd then
            t_object_id:append_text(" (AGENT)")
        elseif object_id_1 == 0xfe then
            t_object_id:append_text(" (CLIENT)")
        elseif object_id_1 == 0xff then
            t_object_id:append_text(" (SESSION)")
        end
    end
    offset = offset + 2

    return offset
end

function idl_xrce_cookie(tree, tvb, offset, encoding, label)
    label = label or "xrce_cookie:"

    tree:add(tvb(offset, 4), label, tvb(offset, 4):string())
    offset = offset + 4

    return offset
end

function idl_xrce_version(tree, tvb, offset, encoding, label)
    label = label or "xrce_version:"

    local xrce_version_major = tvb(offset, 1):uint()
    local xrce_version_minor = tvb(offset + 1, 1):uint()
    local xrce_version = tostring(xrce_version_major) .. "." .. tostring(xrce_version_minor)
    local t_xrce_version = tree:add(tvb(offset, 2), label, xrce_version)
    t_xrce_version:add(tvb(offset, 1), "major:", xrce_version_major)
    t_xrce_version:add(tvb(offset + 1, 1), "minor:", xrce_version_minor)
    offset = offset + 2

    return offset
end

function idl_xrce_vendor_id(tree, tvb, offset, encoding, label)
    label = label or "xrce_vendor_id:"

    local xrce_vendor_id_major = tvb(offset, 1):uint()
    local xrce_vendor_id_minor = tvb(offset + 1, 1):uint()
    local xrce_vendor_id = tostring(xrce_vendor_id_major) .. "." .. tostring(xrce_vendor_id_minor)
    if xrce_vendor_id_major == 0 and xrce_vendor_id_minor == 0 then
        xrce_vendor_id = xrce_vendor_id .. " (INVALID)"
    elseif xrce_vendor_id_major == 1 and xrce_vendor_id_minor == 15 then
        xrce_vendor_id = xrce_vendor_id .. " (eProsima)"
    end
    tree:add(tvb(offset, 2), label, xrce_vendor_id)
    offset = offset + 2

    return offset
end

function idl_time_t(tree, tvb, offset, encoding, label)
    label = label or "timestamp:"

    offset = align(offset, 4)

    local seconds = tvb_int(tvb(offset, 4), encoding)
    local nanoseconds = tvb_uint(tvb(offset, 4), encoding)
    local t_time_t = tree:add(tvb(offset, 8), label, seconds + 1e-9 * nanoseconds)
    t_time_t:add(tvb(offset, 4), "seconds:", seconds)
    offset = offset + 4
    t_time_t:add(tvb(offset, 4), "nanoseconds:", nanoseconds)
    offset = offset + 4

    return offset
end

function idl_session_id(tree, tvb, offset, encoding, label)
    label = label or "session_id:"

    local session_id = tvb(offset, 1):uint()
    local t_session_id = tree:add(tvb(offset, 1), label, tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    if session_id == 0x00 then
        t_session_id:append_text(" (SESSION_ID_NONE_WITH_CLIENT_KEY)")
    elseif session_id == 0x80 then
        t_session_id:append_text(" (SESSION_ID_NONE_WITHOUT_CLIENT_KEY)")
    end
    offset = offset + 1

    return offset
end

function idl_stream_id(tree, tvb, offset, encoding, label)
    label = label or "stream_id:"

    local stream_id = tvb(offset, 1):uint()
    local t_stream_id = tree:add(tvb(offset, 1), label, tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    if stream_id == 0x00 then
        t_stream_id:append_text(" (STREAMID_NONE)")
    elseif session_id == 0x01 then
        t_stream_id:append_text(" (STREAMID_BUILTIN_BEST_EFFORTS)")
    elseif session_id == 0x80 then
        t_stream_id:append_text(" (STREAMID_BUILTIN_RELIABLE)")
    end
    if stream_id >= 0x80 then
        t_stream_id:add(tvb(offset, 1), "reliability:", "RELIABLE")
    elseif stream_id >= 0x01 then
        t_stream_id:add(tvb(offset, 1), "reliability:", "BEST_EFFORTS")
    end
    offset = offset + 1

    return offset
end

function idl_transport_locator(tree, tvb, offset, encoding, label)
    label = label or "address"

    local begin = offset

    local t_transport_locator = tree:add(tvb(offset), label)

    local transport_locator_format = tvb(offset, 1):uint()
    offset = offset + 1

    if transport_locator_format == 0x00 then
        -- ADDRES_FORMAT_SMALL
        tree:add(tvb(offset, 2), "address:", tvb(offset, 2):bytes():tohex(true))
        offset = offset + 2
        t_transport_locator:add(tvb(offset, 1), "locator_port:", tvb_uint(tvb(offset, 1), encoding))
        offset = offset + 1
    elseif transport_locator_format == 0x01 then
        -- ADDRES_FORMAT_MEDIUM
        t_transport_locator:add(
            tvb(offset, 4),
            "address:",
            string.format("%u.%u.%u.%u", tvb(offset, 1), tvb(offset + 1, 1), tvb(offset + 2, 1), tvb(offset + 3, 1))
        )
        offset = offset + 4
        offset = align(offset, 2)
        t_transport_locator:add(tvb(offset, 2), "locator_port:", tvb_uint(tvb(offset, 2), encoding))
        offset = offset + 2
    elseif transport_locator_format == 0x02 then
        -- ADDRES_FORMAT_LARGE
        address_0 = tvb_uint(tvb(offset, 2), encoding)
        address_1 = tvb_uint(tvb(offset + 2, 2), encoding)
        address_2 = tvb_uint(tvb(offset + 4, 2), encoding)
        address_3 = tvb_uint(tvb(offset + 6, 2), encoding)
        address_4 = tvb_uint(tvb(offset + 8, 2), encoding)
        address_5 = tvb_uint(tvb(offset + 10, 2), encoding)
        address_6 = tvb_uint(tvb(offset + 12, 2), encoding)
        address_7 = tvb_uint(tvb(offset + 14, 2), encoding)
        t_transport_locator:add(
            tvb(offset, 16),
            "address:",
            string.format(
                "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                address_0,
                address_1,
                address_2,
                address_3,
                address_4,
                address_5,
                address_6,
                address_7
            )
        )
        offset = offset + 16
        offset = align(offset, 4)
        t_transport_locator:add(tvb(offset, 4), "locator_port:", tvb_uint(tvb(offset, 4), encoding))
        offset = offset + 4
    elseif transport_locator_format == 0x03 then
        -- ADDRES_FORMAT_STRING
        offset = align(offset, 4)
        local size = tvb_uint(tvb(offset, 4), encoding)
        offset = offset + 4
        t_transport_locator:add(tvb(offset, size), "string_locator:", tvb(offset, size):stringz())
        offset = offset + size
    end

    t_transport_locator:set_len(offset - begin)

    return offset
end

function idl_transport_locator_seq(tree, tvb, offset, encoding, label)
    label = label or "address_seq"

    offset = align(offset, 4)
    local begin = offset

    local t_address_seq = tree:add(tvb(offset), label)

    local size = tvb_uint(tvb(offset, 4), encoding)
    offset = offset + 4

    if size == 0 then
        t_address_seq:append_text(" [empty]")
    else
        for i = 1, size do
            offset = idl_transport_locator(t_address_seq, tvb, offset, encoding, "[" .. tostring(i - 1) .. "]")
        end
    end

    t_address_seq:set_len(offset - begin)

    return offset
end

function idl_property(tree, tvb, offset, encoding)
    offset = align(offset, 4)
    local begin = offset
    local name_size = tvb_uint(tvb(offset, 4), encoding)
    offset = offset + 4
    local name = tvb(offset, name_size):stringz()
    offset = offset + name_size

    offset = align(offset, 4)
    local value_size = tvb_uint(tvb(offset, 4), encoding)
    offset = offset + 4
    local value = tvb(offset, value_size):stringz()
    offset = offset + value_size

    tree:add(tvb(begin, offset - begin), name .. ":", value)

    return offset
end

function idl_property_seq(tree, tvb, offset, encoding, label)
    label = label or "properties"

    offset = align(offset, 4)
    local begin = offset

    local t_properties = tree:add(tvb(offset), label)

    local size = tvb_uint(tvb(offset, 4), encoding)
    offset = offset + 4

    if size == 0 then
        t_properties:append_text(" [empty]")
    else
        for i = 1, size do
            offset = idl_property(t_properties, tvb, offset, encoding)
        end
    end

    t_properties:set_len(offset - begin)

    return offset
end

function idl_objk_representation_base(tree, tvb, offset, encoding)
    local format = tvb(offset, 1):uint()
    local t_format = tree:add(tvb(offset, 1), "format:", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    offset = offset + 1

    if format == 0x01 then
        --- REPRESENTATION_BY_REFERENCE
        t_format:append_text(" (REPRESENTATION_BY_REFERENCE)")
        offset = align(offset, 4)
        local size = tvb_uint(tvb(offset, 4), encoding)
        offset = offset + 4
        tree:add(tvb(offset, size), "object_reference:", tvb(offset, size):stringz())
        offset = offset + size
    elseif format == 0x02 then
        --- REPRESENTATION_AS_XML_STRING
        t_format:append_text(" (REPRESENTATION_BY_REFERENCE)")
        offset = align(offset, 4)
        local size = tvb_uint(tvb(offset, 4), encoding)
        offset = offset + 4
        tree:add(tvb(offset, size), "xml_string_representation:", tvb(offset, size):stringz())
        offset = offset + size
    elseif format == 0x03 then
        --- REPRESENTATION_IN_BINARY
        t_format:append_text(" (REPRESENTATION_BY_REFERENCE)")
        offset = align(offset, 4)
        local size = tvb_uint(tvb(offset, 4), encoding)
        offset = offset + 4
        tree:add(tvb(offset, size), "binary_representation:", tvb(offset, size):bytes():tohex(true))
        offset = offset + size
    end

    return offset
end

function idl_objk_qos_profile_representation(tree, tvb, offset, encoding, label)
    label = label or "qos_profile"

    local begin = offset

    local t_qos_profile = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_qos_profile, tvb, offset, encoding)

    t_qos_profile:set_len(offset - begin)

    return offset
end

function idl_objk_type_representation(tree, tvb, offset, encoding, label)
    label = label or "type"

    local begin = offset

    local t_type = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_type, tvb, offset, encoding)

    t_type:set_len(offset - begin)

    return offset
end

function idl_objk_application_representation(tree, tvb, offset, encoding, label)
    label = label or "application"

    local begin = offset

    local t_application = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_application, tvb, offset, encoding)

    t_application:set_len(offset - begin)

    return offset
end

function idl_objk_publisher_representation(tree, tvb, offset, encoding, label)
    label = label or "publisher"

    local begin = offset

    local t_publisher = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_publisher, tvb, offset, encoding)
    offset = idl_object_id(t_publisher, tvb, offset, encoding, "participant_id:")

    t_publisher:set_len(offset - begin)

    return offset
end

function idl_objk_subscriber_representation(tree, tvb, offset, encoding, label)
    label = label or "subscriber"

    local begin = offset

    local t_subscriber = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_subscriber, tvb, offset, encoding)
    offset = idl_object_id(t_subscriber, tvb, offset, encoding, "participant_id:")

    t_subscriber:set_len(offset - begin)

    return offset
end

function idl_objk_data_writer_representation(tree, tvb, offset, encoding, label)
    label = label or "data_writer"

    local begin = offset

    local t_data_writer = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_data_writer, tvb, offset, encoding)
    offset = idl_object_id(t_data_writer, tvb, offset, encoding, "publisher_id:")

    t_data_writer:set_len(offset - begin)

    return offset
end

function idl_objk_data_reader_representation(tree, tvb, offset, encoding, label)
    label = label or "data_reader"

    local begin = offset

    local t_data_reader = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_data_reader, tvb, offset, encoding)
    offset = idl_object_id(t_data_reader, tvb, offset, encoding, "subscriber_id:")

    t_data_reader:set_len(offset - begin)

    return offset
end

function idl_objk_participant_representation(tree, tvb, offset, encoding, label)
    label = label or "participant"

    local begin = offset

    local t_participant = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_participant, tvb, offset, encoding)

    offset = align(offset, 2)
    t_participant:add(tvb(offset, 2), "domain_id:", tvb_int(tvb(offset, 2), encoding))
    offset = offset + 2

    t_participant:set_len(offset - begin)

    return offset
end

function idl_objk_topic_representation(tree, tvb, offset, encoding, label)
    label = label or "topic"

    local begin = offset

    local t_topic = tree:add(tvb(offset), label)

    offset = idl_objk_representation_base(t_topic, tvb, offset, encoding)
    offset = idl_object_id(t_topic, tvb, offset, encoding, "participant_id:")

    t_topic:set_len(offset - begin)

    return offset
end

function idl_object_variant(tree, tvb, offset, encoding, label)
    label = label or "object_variant"

    local begin = offset

    local t_object_variant = tree:add(tvb(offset), label)

    local kind = tvb(offset, 1):uint()
    local t_kind = t_object_variant:add(tvb(offset, 1), "kind:", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    offset = offset + 1

    if kind == 0x0D then
        -- OBJK_AGENT
        t_kind:append_text(" (OBJK_AGENT)")
        offset = idl_agent_representation(t_object_variant, tvb, offset, encoding, "agent")
    elseif kind == 0x0E then
        -- OBJK_CLIENT
        t_kind:append_text(" (OBJK_CLIENT)")
        offset = idl_client_representation(t_object_variant, tvb, offset, encoding, "client")
    elseif kind == 0x0C then
        -- OBJK_APPLICATION
        t_kind:append_text(" (OBJK_APPLICATION)")
        offset = idl_objk_application_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x01 then
        -- OBJK_PARTICIPANT
        t_kind:append_text(" (OBJK_PARTICIPANT)")
        offset = idl_objk_participant_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x0B then
        -- OBJK_QOSPROFILE
        t_kind:append_text(" (OBJK_QOSPROFILE)")
        offset = idl_objk_qos_profile_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x0A then
        -- OBJK_TYPE
        t_kind:append_text(" (OBJK_TYPE)")
        offset = idl_objk_type_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x02 then
        -- OBJK_TOPIC
        t_kind:append_text(" (OBJK_TOPIC)")
        offset = idl_objk_topic_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x03 then
        -- OBJK_PUBLISHER
        t_kind:append_text(" (OBJK_PUBLISHER)")
        offset = idl_objk_publisher_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x04 then
        -- OBJK_SUBSCRIBER
        t_kind:append_text(" (OBJK_SUBSCRIBER)")
        offset = idl_objk_subscriber_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x05 then
        -- OBJK_DATAWRITER
        t_kind:append_text(" (OBJK_DATAWRITER)")
        offset = idl_objk_data_writer_representation(t_object_variant, tvb, offset, encoding)
    elseif kind == 0x06 then
        -- OBJK_DATAREADER
        t_kind:append_text(" (OBJK_DATAREADER)")
        offset = idl_objk_data_reader_representation(t_object_variant, tvb, offset, encoding)
    end

    t_object_variant:set_len(offset - begin)

    return offset
end

function idl_request_id(tree, tvb, offset, encoding, label)
    label = label or "request_id:"

    tree:add(tvb(offset, 2), label, tvb(offset, 2):bytes():tohex(true))
    offset = offset + 2

    return offset
end

function idl_result_status(tree, tvb, offset, encoding, label)
    label = label or "result_status"

    local t_result_status = tree:add(tvb(offset, 2), label)

    local status = tvb_uint(tvb(offset, 1), encoding)
    local t_status = t_result_status:add(tvb(offset, 1), "status", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    if status == 0x00 then
        t_status:append_text(" (STATUS_OK)")
    elseif status == 0x01 then
        t_status:append_text(" (STATUS_OK_MATCHED)")
    elseif status == 0x80 then
        t_status:append_text(" (STATUS_ERR_DDS_ERROR)")
    elseif status == 0x81 then
        t_status:append_text(" (STATUS_ERR_MISMATCH)")
    elseif status == 0x82 then
        t_status:append_text(" (STATUS_ERR_ALREADY_EXISTS)")
    elseif status == 0x83 then
        t_status:append_text(" (STATUS_ERR_DENIED)")
    elseif status == 0x84 then
        t_status:append_text(" (STATUS_ERR_UNKNOWN_REFERENEC)")
    elseif status == 0x85 then
        t_status:append_text(" (STATUS_ERR_INVALID_DATA)")
    elseif status == 0x86 then
        t_status:append_text(" (STATUS_ERR_INCOMPATIBLE)")
    elseif status == 0x87 then
        t_status:append_text(" (STATUS_ERR_RESOURCES)")
    end
    offset = offset + 1

    t_result_status:add(tvb(offset, 1), "implementation_status", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    offset = offset + 1

    return offset
end

function idl_info_mask(tree, tvb, offset, encoding, label)
    label = label or "info_mask:"

    local info_mask = tvb_uint(tvb(offset, 4), encoding)
    local t_info_mask = tree:add(tvb(offset, 1), label, tvb_format_uint(tvb(offset, 4), encoding, base.HEX))
    if bit.band(info_mask, 0x2) == 0x2 then
        t_info_mask:append_text(", INFO_ACTIVITY")
        t_info_mask:add(tvb(offset, 4), ".... .... .... .... .... .... .... ..1. = INFO_ACTIVITY:", "Set")
    else
        t_info_mask:add(tvb(offset, 4), ".... .... .... .... .... .... .... ..0. = INFO_ACTIVITY:", "Not set")
    end
    if bit.band(info_mask, 0x1) == 0x1 then
        t_info_mask:append_text(", INFO_CONFIGURATION")
        t_info_mask:add(tvb(offset, 4), ".... .... .... .... .... .... .... ...1 = INFO_CONFIGURATION:", "Set")
    else
        t_info_mask:add(tvb(offset, 4), ".... .... .... .... .... .... .... ...0 = INFO_CONFIGURATION:", "Not set")
    end
    offset = offset + 4

    return offset
end

function idl_object_info(tree, tvb, offset, encoding, label)
    label = label or "object_info"

    local begin = offset

    local t_object_info = tree:add(tvb(offset), label)

    local has_config = tvb(offset, 1):uint()
    offset = offset + 1
    if has_config > 0 then
        offset = idl_object_variant(t_object_info, tvb, offset, encoding, "config")
    end

    local has_activity = tvb(offset, 1):uint()
    offset = offset + 1
    if has_activity > 0 then
        local begin_activity = offset
        local t_activity = t_object_info:add(tvb(offset), "activity")

        -- ActivityInfoVariant
        local kind = tvb(offset, 1):uint()
        local t_kind = t_activity:add(tvb(offset, 1), "kind:", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
        offset = offset + 1
        if kind == 0x0D then
            -- OBJK_AGENT
            t_kind:append_text(" (OBJK_AGENT)")
            offset = align(offset, 2)
            local begin_agent = offset
            local t_agent = t_activity:add(tvb(offset, 2), "agent")
            t_agent:add(tvb(offset, 2), "availability:", tvb_int(tvb(offset, 2), encoding))
            offset = offset + 2
            offset = idl_transport_locator_seq(t_agent, tvb, offset, encoding)
            t_agent:set_len(offset - begin_agent)
        elseif kind == 0x05 then
            -- OBJK_DATAWRITER
            t_kind:append_text(" (OBJK_DATAWRITER)")
            offset = align(offset, 8)
            local t_data_writer = t_activity:add(tvb(offset, 2), "data_writer")
            t_data_reader:add(tvb(offset, 8), "sample_seq_num:", tvb_int(tvb(offset, 8), encoding))
            offset = offset + 8
            t_data_reader:add(tvb(offset, 2), "stream_seq_num:", tvb_int(tvb(offset, 2), encoding))
            offset = offset + 2
        elseif kind == 0x06 then
            -- OBJK_DATAREADER
            t_kind:append_text(" (OBJK_DATAREADER)")
            offset = align(offset, 2)
            local t_data_reader = t_activity:add(tvb(offset, 2), "data_reader")
            t_data_reader:add(tvb(offset, 2), "highest_acked_num:", tvb_int(tvb(offset, 2), encoding))
            offset = offset + 2
        end

        t_activity:set_len(offset - begin_activity)
    end

    t_object_info:set_len(offset - begin)

    return offset
end

function idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_request_id(tree, tvb, offset, encoding)
    offset = idl_object_id(tree, tvb, offset, encoding)

    return offset
end

function idl_base_object_reply(tree, tvb, offset, encoding)
    local begin = offset

    local t_related_request = tree:add(tvb(offset), "related_request")
    offset = idl_base_object_request(t_related_request, tvb, offset, encoding)
    t_related_request:set_len(offset - begin)

    offset = idl_result_status(tree, tvb, offset, encoding)

    return offset
end

function idl_data_format(tree, tvb, offset, encoding, label)
    label = label or "data_format:"

    local data_format = tvb(offset, 1):uint()
    local t_data_format = tree:add(tvb(offset, 1), label, tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    if data_format == 0x00 then
        t_data_format:append_text(" (FORMAT_DATA)")
    elseif session_id == 0x02 then
        t_data_format:append_text(" (FORMAT_SAMPLE)")
    elseif session_id == 0x08 then
        t_data_format:append_text(" (FORMAT_DATA_SEQ)")
    elseif session_id == 0x0A then
        t_data_format:append_text(" (FORMAT_SAMPLE_SEQ)")
    elseif session_id == 0x0E then
        t_data_format:append_text(" (FORMAT_PACKED_SAMPLES)")
    end
    offset = offset + 1

    return offset
end

function idl_read_specification(tree, tvb, offset, encoding, label)
    label = label or "read_specification"

    local begin = offset

    local t_read_specification = tree:add(tvb(offset), label)

    offset = idl_stream_id(t_read_specification, tvb, offset, encoding, "preferred_stream_id:")
    offset = idl_data_format(t_read_specification, tvb, offset, encoding)

    local has_content_filter_expression = tvb(offset, 1):uint()
    offset = offset + 1
    if has_content_filter_expression > 0 then
        offset = align(offset, 4)
        local size = tvb_uint(tvb(offset, 4), encoding)
        offset = offset + 4
        t_read_specification:add(tvb(offset, size), "content_filter_expression:", tvb(offset, size):stringz())
        offset = offset + size
    end

    local has_delivery_control = tvb(offset, 1):uint()
    offset = offset + 1
    if has_delivery_control > 0 then
        offset = align(offset, 2)
        local t_delivery_control = t_read_specification:add(tvb(offset, 8), "delivery_control")
        t_delivery_control:add(tvb(offset, 2), "max_samples:", tvb_uint(tvb(offset, 2), encoding))
        offset = offset + 2
        t_delivery_control:add(tvb(offset, 2), "max_elapsed_time:", tvb_uint(tvb(offset, 2), encoding))
        offset = offset + 2
        t_delivery_control:add(tvb(offset, 2), "max_bytes_per_second:", tvb_uint(tvb(offset, 2), encoding))
        offset = offset + 2
        t_delivery_control:add(tvb(offset, 2), "min_pace_period:", tvb_uint(tvb(offset, 2), encoding))
        offset = offset + 2
    end

    t_read_specification:set_len(offset - begin)

    return offset
end

function idl_sample_info_flags(tree, tvb, offset, encoding, label)
    label = label or "state"

    local samsample_info_flagsple_info_flags = tvb(offset, 1):uint()
    local t_sample_info_flags = tree:add(tvb(offset, 1), label, tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    if bit.band(sample_info_flags, 0x8) == 0x8 then
        t_sample_info_flags:append_text(", SAMPLE_STATE_READ")
        t_sample_info_flags:add(tvb(offset, 1), ".... 1... = SAMPLE_STATE_READ:", "Set")
    else
        t_sample_info_flags:add(tvb(offset, 1), ".... 0... = SAMPLE_STATE_READ:", "Not set")
    end
    if bit.band(sample_info_flags, 0x4) == 0x4 then
        t_sample_info_flags:append_text(", VIEW_STATE_NEW")
        t_sample_info_flags:add(tvb(offset, 1), ".... .1.. = VIEW_STATE_NEW:", "Set")
    else
        t_sample_info_flags:add(tvb(offset, 1), ".... .0.. = VIEW_STATE_NEW:", "Not set")
    end
    if bit.band(sample_info_flags, 0x2) == 0x2 then
        t_sample_info_flags:append_text(", INSTANCE_STATE_DISPOSED")
        t_sample_info_flags:add(tvb(offset, 1), ".... ..1. = INSTANCE_STATE_DISPOSED:", "Set")
    else
        t_sample_info_flags:add(tvb(offset, 1), ".... ..0. = INSTANCE_STATE_DISPOSED:", "Not set")
    end
    if bit.band(sample_info_flags, 0x1) == 0x1 then
        t_sample_info_flags:append_text(", INSTANCE_STATE_UNREGISTERED")
        t_sample_info_flags:add(tvb(offset, 1), ".... ...1 = INSTANCE_STATE_UNREGISTERED:", "Set")
    else
        t_sample_info_flags:add(tvb(offset, 1), ".... ...0 = INSTANCE_STATE_UNREGISTERED:", "Not set")
    end
    offset = offset + 1

    return offset
end

function idl_sample_info(tree, tvb, offset, encoding, label)
    label = label or "info"

    local begin = offset

    local t_sample_info = tree:add(tvb(offset), label)

    offset = idl_sample_info_flags(t_sample_info, tvb, offset, encoding)

    -- SampleInfoDetail
    local format = tvb(offset, 1):uint()
    local t_format = t_sample_info:add(tvb(offset, 1), "format:", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    offset = offset + 1
    if format == 0x00 then
        -- FORMAT_EMPTY
        t_format:append_text(" (FORMAT_EMPTY)")
    elseif format == 0x01 then
        -- FORMAT_SEQNUM
        t_format:append_text(" (FORMAT_SEQNUM)")
        offset = align(offset, 4)
        t_sample_info:add(tvb(offset, 4), "sequence_number:", tvb_uint(tvb(offset, 4), encoding))
        offset = offset + 4
    elseif format == 0x02 then
        -- FORMAT_TIMESTAMP
        t_format:append_text(" (FORMAT_TIMESTAMP)")
        offset = align(offset, 4)
        t_sample_info:add(tvb(offset, 4), "session_time_offset:", tvb_uint(tvb(offset, 4), encoding))
        offset = offset + 4
    elseif format == 0x03 then
        -- FORMAT_SEQN_TIMS
        t_format:append_text(" (FORMAT_SEQN_TIMS)")
        offset = align(offset, 4)
        local t_seqnum_n_timestamp = t_sample_info:add(tvb(offset, 8), "seqnum_n_timestamp")
        t_seqnum_n_timestamp:add(tvb(offset, 4), "sequence_number:", tvb_uint(tvb(offset, 4), encoding))
        offset = offset + 4
        t_seqnum_n_timestamp:add(tvb(offset, 4), "session_time_offset:", tvb_uint(tvb(offset, 4), encoding))
        offset = offset + 4
    end

    t_sample_info:set_len(offset - begin)

    return offset
end

function idl_sample_data(tree, tvb, offset, encoding, label)
    label = label or "data"

    tree:add(tvb(offset), label):add(tvb(offset), "serialized_data:", tvb(offset):bytes():tohex(true, " "))
    offset = tvb:len()

    return offset
end

function idl_sample_data_seq(tree, tvb, offset, encoding, label)
    label = label or "data_seq"

    offset = align(offset, 4)
    local begin = offset

    local t_sample_data_seq = tree:add(tvb(offset), label)

    local size = tvb_uint(tvb(offset, 4), encoding)
    offset = offset + 4

    if size == 0 then
        t_sample_data_seq:append_text(" [empty]")
    else
        local sample_data_size = math.ceil((tvb:len() - offset) / size)
        for i = 1, size do
            offset =
                idl_sample_data(
                t_sample_data_seq,
                tvb(0, offset + sample_data_size),
                offset,
                encoding,
                "[" .. tostring(i - 1) .. "]"
            )
        end
    end

    t_sample_data_seq:set_len(offset - begin)

    return offset
end

function idl_sample_seq(tree, tvb, offset, encoding, label)
    label = label or "sample_seq"

    offset = align(offset, 4)
    local begin = offset

    local t_sample_seq = tree:add(tvb(offset), label)

    local size = tvb_uint(tvb(offset, 4), encoding)
    offset = offset + 4

    if size == 0 then
        t_sample_seq:append_text(" [empty]")
    else
        local sample_size = math.ceil((tvb:len() - offset) / size)
        for i = 1, size do
            offset =
                idl_sample(t_sample_seq, tvb(0, offset + sample_size), offset, encoding, "[" .. tostring(i - 1) .. "]")
        end
    end

    t_sample_seq:set_len(offset - begin)

    return offset
end

function idl_sample(tree, tvb, offset, encoding, label)
    label = label or "sample"

    local begin = offset

    local t_sample = tree:add(tvb(offset), label)

    offset = idl_sample_info(t_sample, tvb, offset, encoding)
    offset = idl_sample_data(t_sample, tvb, offset, encoding)

    t_sample:set_len(offset - begin)

    return offset
end

function idl_sample_delta(tree, tvb, offset, encoding, label)
    label = label or "idl_sample_delta"

    local begin = offset

    local t_sample_delta = tree:add(tvb(offset), label)

    -- SampleInfoDelta
    local t_sample_info_delta = t_sample_delta:add(tvb(offset, align(offset + 2, 2) + 2), "info_delta")
    offset = t_sample_info_flags(t_sample_info_delta, tvb, offset, encoding)
    t_sample_info_delta:add(tvb(offset, 1), "seq_number_delta:", tvb_int(tvb(offset, 1), encoding))
    offset = offset + 1
    offset = align(offset, 2)
    t_sample_info_delta:add(tvb(offset, 1), "timestamp_delta:", tvb_uint(tvb(offset, 2), encoding))
    offset = offset + 2

    offset = idl_sample_data(tree, tvb, offset, encoding)

    t_sample_delta:set_len(offset - begin)

    return offset
end

function idl_packed_samples(tree, tvb, offset, encoding, label)
    label = label or "packed_samples"

    local begin = offset

    local t_packed_samples = tree:add(tvb(offset), label)

    offset = t_sample_info(t_packed_samples, tvb, offset, encoding, "info_base")

    offset = align(offset, 4)
    local begin_sample_delta_seq = offset

    local t_sample_delta_seq = tree:add(tvb(offset), "sample_delta_seq")

    local size = tvb_uint(tvb(offset, 4), encoding)
    offset = offset + 4

    if size == 0 then
        t_sample_delta_seq:append_text(" [empty]")
    else
        local sample_delta_size = math.ceil((tvb:len() - offset) / size)
        for i = 1, size do
            offset =
                idl_sample_delta(
                t_sample_seq,
                tvb(0, offset + sample_delta_size),
                offset,
                encoding,
                "[" .. tostring(i - 1) .. "]"
            )
        end
    end

    t_sample_delta_seq:set_len(offset - begin_sample_delta_seq)

    t_packed_samples:set_len(offset - begin)

    return offset
end

function idl_client_representation(tree, tvb, offset, encoding, label)
    label = label or "client_representation"

    local begin = offset

    local t_client_representation = tree:add(tvb(offset), label)
    offset = idl_xrce_cookie(t_client_representation, tvb, offset, encoding)
    offset = idl_xrce_version(t_client_representation, tvb, offset, encoding)
    offset = idl_xrce_vendor_id(t_client_representation, tvb, offset, encoding)
    offset = idl_client_key(t_client_representation, tvb, offset, encoding)
    offset = idl_session_id(t_client_representation, tvb, offset, encoding)
    local has_properties = tvb(offset, 1):uint()
    offset = offset + 1
    if has_properties > 0 then
        offset = idl_property_seq(t_client_representation, tvb, offset, encoding)
    end
    t_client_representation:set_len(offset - begin)

    return offset
end

function idl_agent_representation(tree, tvb, offset, encoding, label)
    label = label or "agent_representation"

    local begin = offset

    local t_agent_representation = tree:add(tvb(offset), label)
    offset = idl_xrce_cookie(t_agent_representation, tvb, offset, encoding)
    offset = idl_xrce_version(t_agent_representation, tvb, offset, encoding)
    offset = idl_xrce_vendor_id(t_agent_representation, tvb, offset, encoding)
    local has_properties = tvb(offset, 1):uint()
    offset = offset + 1
    if has_properties > 0 then
        offset = idl_property_seq(t_agent_representation, tvb, offset, encoding)
    end
    t_agent_representation:set_len(offset - begin)

    return offset
end

function idl_create_client_payload(tree, tvb, offset, encoding)
    return idl_client_representation(tree, tvb, offset, encoding)
end

function idl_create_payload(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_object_variant(tree, tvb, offset, encoding, "object_representation")

    return offset
end

function idl_get_info_payload(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_info_mask(tree, tvb, offset, encoding)

    return offset
end

function idl_delete_payload(tree, tvb, offset, encoding)
    return idl_base_object_request(tree, tvb, offset, encoding)
end

function idl_status_agent_payload(tree, tvb, offset, encoding)
    offset = idl_result_status(tree, tvb, offset, encoding, "result")
    offset = idl_agent_representation(tree, tvb, offset, encoding, "agent_info")

    return offset
end

function idl_status_payload(tree, tvb, offset, encoding)
    return idl_base_object_reply(tree, tvb, offset, encoding)
end

function idl_info_payload(tree, tvb, offset, encoding)
    offset = idl_base_object_reply(tree, tvb, offset, encoding)
    offset = idl_object_info(tree, tvb, offset, encoding)

    return offset
end

function idl_read_data_payload(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_read_specification(tree, tvb, offset, encoding)

    return offset
end

function idl_write_data_payload_data(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample_data(tree, tvb, offset, encoding)

    return offset
end

function idl_write_data_payload_sample(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample(tree, tvb, offset, encoding)

    return offset
end

function idl_write_data_payload_data_seq(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample_data_seq(tree, tvb, offset, encoding)

    return offset
end

function idl_write_data_payload_sample_seq(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample_seq(tree, tvb, offset, encoding)

    return offset
end

function idl_write_data_payload_packed_samples(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_packed_samples(tree, tvb, offset, encoding)

    return offset
end

function idl_data_payload_data(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample_data(tree, tvb, offset, encoding)

    return offset
end

function idl_data_payload_sample(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample(tree, tvb, offset, encoding)

    return offset
end

function idl_data_payload_data_seq(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample_data_seq(tree, tvb, offset, encoding)

    return offset
end

function idl_data_payload_sample_seq(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_sample_seq(tree, tvb, offset, encoding)

    return offset
end

function idl_data_payload_packed_samples(tree, tvb, offset, encoding)
    offset = idl_base_object_request(tree, tvb, offset, encoding)
    offset = idl_packed_samples(tree, tvb, offset, encoding)

    return offset
end

function idl_acknack_payload(tree, tvb, offset, encoding)
    offset = align(offset, 2)

    tree:add(tvb(offset, 2), "first_unacked_seq_num:", tvb_format_uint(tvb(offset, 2), encoding, base.DEC))
    offset = offset + 2

    tree:add(tvb(offset, 2), "nack_bitmap:", tvb_format_uint(tvb(offset, 2), encoding, base.HEX))
    offset = offset + 2

    tree:add(tvb(offset, 1), "stream_id:", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    offset = offset + 1

    return offset
end

function idl_heartbeat_payload(tree, tvb, offset, encoding)
    offset = align(offset, 2)

    tree:add(tvb(offset, 2), "first_unacked_seq_nr:", tvb_format_uint(tvb(offset, 2), encoding, base.DEC))
    offset = offset + 2

    tree:add(tvb(offset, 2), "last_unacked_seq_nr:", tvb_format_uint(tvb(offset, 2), encoding, base.DEC))
    offset = offset + 2

    tree:add(tvb(offset, 1), "stream_id:", tvb_format_uint(tvb(offset, 1), encoding, base.HEX))
    offset = offset + 1

    return offset
end

function idl_timestamp_payload(tree, tvb, offset, encoding)
    return idl_time_t(tree, tvb, offset, encoding, "transmit_timestamp:")
end

function idl_timestamp_reply_payload(tree, tvb, offset, encoding)
    offset = idl_time_t(tree, tvb, offset, encoding, "transmit_timestamp:")
    offset = idl_time_t(tree, tvb, offset, encoding, "receive_timestamp:")
    offset = idl_time_t(tree, tvb, offset, encoding, "originate_timestamp:")

    return offset
end