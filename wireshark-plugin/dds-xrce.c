#include "config.h"
#include <epan/packet.h>

#define XRCE_PORT 8888

static int proto_dds_xrce = -1;

static int header_session_id = -1;
static int header_stream_id = -1;
static int header_sequence_no = -1;
static int header_client_key = -1;

static int subheader_submessage_id = -1;
static int subheader_flags= -1;
static int subheader_submessage_length = -1;
static int subheader_payload = -1;

static int acknack_first_unacked_seq_nr = -1;
static int acknack_last_unacked_seq_nr = -1;
static int acknack_stream_id = -1;

static const value_string subheader_submessage_id_names[] = {
    { 0, "CREATE_CLIENT"},
    { 1, "CREATE"},
    { 2, "GET_INFO"},
    { 3, "DELETE_ID"},
    { 4, "STATUS_AGENT"},
    { 5, "STATUS"},
    { 6, "INFO"},
    { 7, "WRITE_DATA"},
    { 8, "READ_DATA"},
    { 9, "DATA"},
    { 10, "ACKNACK"},
    { 11, "HEARTBEAT"},
    { 12, "RESET"},
    { 13, "FRAGMENT"},
    { 14, "TIMESTAMP"},
    { 15, "TIMESTAMP_REPLY"},
    { 255, "PERFORMANCE"},
};

static gint header_tree_handle = -1;

static int
dissect_dds_xrce(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "DDS-XRCE");
    col_set_str(pinfo->cinfo, COL_INFO, "DDS-XRCE protocol");

    gint offset = 0;

    proto_item *ti = proto_tree_add_item(tree, proto_dds_xrce, tvb, 0, -1, ENC_NA);

    proto_tree *header_tree = proto_item_add_subtree(ti, header_tree_handle);

    proto_tree_add_item(header_tree, header_session_id, tvb, offset, 1, ENC_LITTLE_ENDIAN);
    guint8 session_id = tvb_get_guint8(tvb, offset);
    offset += 1;
    proto_tree_add_item(header_tree, header_stream_id, tvb, offset, 1, ENC_LITTLE_ENDIAN);
    offset += 1;
    proto_tree_add_item(header_tree, header_sequence_no, tvb, offset, 2, ENC_LITTLE_ENDIAN);
    offset += 2;
    if (session_id <= 127)
    {
        proto_tree_add_item(header_tree, header_client_key, tvb, offset, 4, ENC_LITTLE_ENDIAN);
        offset += 4;
    }

    proto_tree_add_item(header_tree, header_session_id, tvb, offset, 1, ENC_LITTLE_ENDIAN);


    while (tvb_offset_exists(tvb, offset))
    {
        proto_item *sub_ti = proto_tree_add_item(header_tree, subheader_submessage_id, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        guint8 submessage_id = tvb_get_guint8(tvb, offset);

        proto_tree *subheader_tree = proto_item_add_subtree(sub_ti, header_tree_handle);
        offset += 1;
        proto_tree_add_item(subheader_tree, subheader_flags, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        offset += 1;
        proto_tree_add_item(subheader_tree, subheader_submessage_length, tvb, offset, 2, ENC_LITTLE_ENDIAN);
        guint16 submessage_length = tvb_get_guint16(tvb, offset, ENC_LITTLE_ENDIAN);
        offset += 2;
        guint8 aux = 0;
        proto_tree_add_bytes(subheader_tree, subheader_payload, tvb, offset, submessage_length, &aux);

        switch (submessage_id)
        {
            case 10: //ACKNACK
            {
                guint8 inner_offset = 0;
                proto_tree_add_item(subheader_tree, acknack_first_unacked_seq_nr, tvb, offset + inner_offset, 2, ENC_LITTLE_ENDIAN);
                inner_offset += 2;
                proto_tree_add_item(subheader_tree, acknack_last_unacked_seq_nr, tvb, offset + inner_offset, 2, ENC_LITTLE_ENDIAN);
                inner_offset += 2;
                proto_tree_add_item(subheader_tree, acknack_stream_id, tvb, offset + inner_offset, 1, ENC_LITTLE_ENDIAN);
                break;
            }
            default:
                break;
        }

        offset += submessage_length;

        // Fill padding for aligment
        while (offset % 4 != 0){
            offset++;
        }

    }


    return tvb_captured_length(tvb);
}

void
proto_register_dds_xrce(void)
{
    static hf_register_info hf[] = {
        { &header_session_id,
            { "Session id", "ddsxrce.session_id",
            FT_UINT8, BASE_HEX,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &header_stream_id,
            { "Stream id", "ddsxrce.stream_id",
            FT_UINT8, BASE_HEX,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &header_sequence_no,
            { "Sequence number", "ddsxrce.sequence_no",
            FT_UINT16, BASE_DEC,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &header_client_key,
            { "Client key", "ddsxrce.client_key",
            FT_UINT32, BASE_HEX,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &subheader_submessage_id,
            { "Subheader id", "ddsxrce.subheader.id",
            FT_UINT8, BASE_HEX,
            VALS(subheader_submessage_id_names), 0x0,
            NULL, HFILL }
        },
        { &subheader_flags,
            { "Subheader flags", "ddsxrce.subheader.flags",
            FT_UINT8, BASE_HEX,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &subheader_submessage_length,
            { "Payload lenght", "ddsxrce.subheader.lenght",
            FT_UINT16, BASE_DEC,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &subheader_payload,
            { "Payload", "ddsxrce.subheader.payload",
            FT_BYTES, SEP_SPACE,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &acknack_first_unacked_seq_nr,
            { "First unacked sequence num", "ddsxrce.acknack.first_unack",
            FT_UINT16, BASE_DEC,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &acknack_last_unacked_seq_nr,
            { "Last unacked sequence num", "ddsxrce.acknack.last_unack",
            FT_UINT16, BASE_DEC,
            NULL, 0x0,
            NULL, HFILL }
        },
        { &acknack_stream_id,
            { "Stream id", "ddsxrce.acknack.stream_id",
            FT_UINT8, BASE_HEX,
            NULL, 0x0,
            NULL, HFILL }
        },
    };

    /* Setup protocol subtree array */
    static gint *ett[] = {
        &header_tree_handle,
    };

    proto_dds_xrce = proto_register_protocol (
        "DDS-XRCE Protocol", /* name        */
        "DDS-XRCE",          /* short_name  */
        "ddsxrce"           /* filter_name */
        );

    proto_register_field_array(proto_dds_xrce, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
}

void
proto_reg_handoff_dds_xrce(void)
{
    static dissector_handle_t dds_xrce_handle;

    dds_xrce_handle = create_dissector_handle(dissect_dds_xrce, proto_dds_xrce);
    dissector_add_uint("udp.port", XRCE_PORT, dds_xrce_handle);
}