-- place in ~/.config/wireshark/plugins/

local p_xrce_dds = Proto("xrce-dds", "XRCE-DDS");
local p_xrce_dds_header = Proto("xrce-dds-header", "Header");
local p_xrce_dds_submessage = Proto("xrce-dds-submessage", "Submessage");

-- Header
local f_sessionId = ProtoField.uint8("xrce-dds.sessionId", "sessionId", base.HEX)
local f_streamId = ProtoField.uint8("xrce-dds.streamId", "streamId", base.HEX)
local f_sequenceNr = ProtoField.uint16("xrce-dds.sequenceNr", "sequenceNr", base.DEC)
local f_clientKey = ProtoField.uint32("xrce-dds.clientKey", "clientKey", base.HEX)

p_xrce_dds_header.fields = { f_sessionId, f_streamId, f_sequenceNr, f_clientKey }

-- Subheader
local f_submessageId = ProtoField.uint8("xrce-dds.submessageId", "submessageId", base.HEX)
local f_flags = ProtoField.uint8("xrce-dds.flags", "flags", base.HEX)
local f_submessageLength = ProtoField.uint16("xrce-dds.submessageLength", "submessageLength", base.DEC)

p_xrce_dds_submessage.fields = { f_submessageId, f_flags, f_submessageLength }


local SUBMESSAGE_ID =  {
    [0] = "CREATE_CLIENT",
    [1] = "CREATE",
    [2] = "GET_INFO",
    [3] = "DELETE_ID",
    [4] = "STATUS_AGENT",
    [5] = "STATUS",
    [6] = "INFO",
    [7] = "WRITE_DATA",
    [8] = "READ_DATA",
    [9] = "DATA",
    [10] = "ACKNACK",
    [11] = "HEARTBEAT",
    [12] = "RESET",
    [13] = "FRAGMENT",
    [14] = "TIMESTAMP",
    [15] = "TIMESTAMP_REPLY",
    [255] = "PERFORMANCE",
}

function p_xrce_dds.dissector(buf, pkt, tree)
        pkt.cols.protocol = "XRCE-DDS"

        if buf(0,4):string() == "RTPS" then
            return 0
        end

        local header_len = 0
        local subtree = tree:add(p_xrce_dds, buf())
        local header = subtree:add(p_xrce_dds_header, buf())
        header:add(f_sessionId, buf(0,1))
        header:add(f_streamId, buf(1,1))
        header:add_le(f_sequenceNr, buf(2,2))
        local session_id = buf(0,1):uint()
        if session_id <= 127 then
            subtree:add(f_clientKey, buf(4,4))
            header_len = 8
        else
            header_len = 4
        end

        print(buf:len())

        local submessage = subtree:add(p_xrce_dds_submessage, buf())
        local submessage_id = buf(header_len,1):uint()
        submessage:add(f_submessageId, buf(header_len, 1)):append_text(" (" .. SUBMESSAGE_ID[submessage_id] .. ")")
        submessage:add(f_flags, buf(header_len + 1, 1))
        submessage:add_le(f_submessageLength, buf(header_len + 2, 2))

        if submessage_id == 0 then


end


local udp_encap_table = DissectorTable.get("udp.port")
udp_encap_table:add(8888, p_xrce_dds)
udp_encap_table:add(2018, p_xrce_dds)
udp_encap_table:add(7400, p_xrce_dds)