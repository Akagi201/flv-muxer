/*
 * @file flv-muxer.c
 * @author Akagi201
 * @date 2015/02/04
 */

#include <stdio.h>
#include <stdlib.h>

#include "flv-muxer.h"
#include "amf-byte-stream.h"

typedef struct flv_tag {
    uint8_t type;
    uint8_t data_size[3];
    uint8_t timestamp[3];
    uint8_t timestamp_ex;
    uint8_t streamid[3];
} flv_tag_t;

static FILE *g_file_handle = NULL;
static uint64_t g_time_begin;

void flv_file_open(const char *filename) {
    if (NULL == filename) {
        return;
    }

    g_file_handle = fopen(filename, "wb");

    return;
}

void write_flv_header(bool is_have_audio, bool is_have_video) {
    char flv_file_header[] = "FLV\x1\x5\0\0\0\x9\0\0\0\0"; // have audio and have video

    if (is_have_audio && is_have_video) {
        flv_file_header[4] = 0x05;
    } else if (is_have_audio && !is_have_video) {
        flv_file_header[4] = 0x04;
    } else if (!is_have_audio && is_have_video) {
        flv_file_header[4] = 0x01;
    } else {
        flv_file_header[4] = 0x00;
    }

    fwrite(flv_file_header, 13, 1, g_file_handle);

    return;
}

void write_video_tag(uint8_t *buf, uint32_t buf_len, uint32_t timestamp) {
    uint8_t prev_size[4] = {0};

    flv_tag_t flvtag;

    memset(&flvtag, 0, sizeof(flvtag));

    flvtag.type = FLV_TAG_TYPE_VIDEO;
    ui24_to_bytes((uint8_t *) flvtag.data_size, buf_len);
    flvtag.timestamp_ex = (uint8_t) ((timestamp >> 24) & 0xff);
    flvtag.timestamp[0] = (uint8_t) ((timestamp >> 16) & 0xff);
    flvtag.timestamp[1] = (uint8_t) ((timestamp >> 8) & 0xff);
    flvtag.timestamp[2] = (uint8_t) ((timestamp) & 0xff);

    fwrite(&flvtag, sizeof(flvtag), 1, g_file_handle);
    fwrite(buf, 1, buf_len, g_file_handle);

    ui32_to_bytes(prev_size, buf_len + (uint32_t) sizeof(flvtag));
    fwrite(prev_size, 4, 1, g_file_handle);

    return;

}

void write_audio_tag(uint8_t *buf, int buf_len, uint32_t timestamp) {

}

void write_avc_data_tag(const uint8_t *data, int data_len, unsigned int timestamp, int is_keyframe) {
    uint8_t *buf = (uint8_t *) malloc(data_len + 5);
    uint8_t *pbuf = buf;

    unsigned char flag = 0;
    if (is_keyframe) {
        flag = 0x17;
    } else {
        flag = 0x27;
    }

    pbuf = ui08_to_bytes(pbuf, flag);

    pbuf = ui08_to_bytes(pbuf, 1);    // avc packet type (0, nalu)
    pbuf = ui24_to_bytes(pbuf, 0);    // composition time

    memcpy(pbuf, data, data_len);
    pbuf += data_len;

    if (g_time_begin == -1) {
        g_time_begin = timestamp;
    }

    if (timestamp < g_time_begin) {
        g_time_begin = 0;
    }

    write_video_tag(buf, (uint32_t) (pbuf - buf), (uint32_t) (timestamp - g_time_begin));

    free(buf);

    return;
}

void write_video_data_tag(const uint8_t *data, int data_len, unsigned int timestamp) {
    if (g_time_begin == -1) {
        g_time_begin = timestamp;
    }

    if (timestamp < g_time_begin) {
        g_time_begin = 0;
    }

    write_video_tag((uint8_t *) data, data_len, (uint32_t) (timestamp - g_time_begin));
}

void write_avc_sequence_header_tag(const uint8_t *sps, int sps_len, const uint8_t *pps, int pps_len) {
    uint8_t avc_seq_buf[4096] = {0};
    uint8_t *pbuf = avc_seq_buf;

    uint8_t flag = 0;

    flag = (1 << 4) // frametype "1 == keyframe"
            | 7; // codecid "7 == AVC"

    pbuf = ui08_to_bytes(pbuf, flag);

    pbuf = ui08_to_bytes(pbuf, 0); // avc packet type (0, header)
    pbuf = ui24_to_bytes(pbuf, 0); // composition time

    // AVCDecoderConfigurationRecord

    pbuf = ui08_to_bytes(pbuf, 1); // configurationVersion
    pbuf = ui08_to_bytes(pbuf, sps[1]); // AVCProfileIndication
    pbuf = ui08_to_bytes(pbuf, sps[2]); // profile_compatibility
    pbuf = ui08_to_bytes(pbuf, sps[3]); // AVCLevelIndication
    pbuf = ui08_to_bytes(pbuf, 0xff); // 6 bits reserved (111111) + 2 bits nal size length - 1
    pbuf = ui08_to_bytes(pbuf, (uint16_t) sps_len); // sps

    memcpy(pbuf, sps, sps_len);

    pbuf += sps_len;
    pbuf = ui08_to_bytes(pbuf, 1); // number of pps
    pbuf = ui16_to_bytes(pbuf, (uint16_t) pps); // pps
    memcpy(pbuf, pps, pps_len);
    pbuf += pps_len;

    write_video_data_tag(avc_seq_buf, (int) (pbuf - avc_seq_buf), 0);

    return;
}