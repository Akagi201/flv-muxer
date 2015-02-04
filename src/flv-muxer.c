/*
 * @file flv-muxer.c
 * @author Akagi201
 * @date 2015/02/04
 */

#include <stdio.h>

#include "flv-muxer.h"

static FILE *g_file_handle = NULL;

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
    } else if (is_have_audio && ! is_have_video) {
        flv_file_header[4] = 0x04;
    } else if (!is_have_audio && is_have_video) {
        flv_file_header[4] = 0x01;
    } else {
        flv_file_header[4] = 0x00;
    }

    fwrite(flv_file_header, 13, 1, g_file_handle);

    return;
}

void write_avc_sequence_header_tag(const char *sps, int sps_len, const char *pps, int pps_len) {

}