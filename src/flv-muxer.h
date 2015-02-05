/*
 * @file flv-muxer.h
 * @author Akagi201
 * @date 2015/02/04
 */

#ifndef FLV_MUXER_H_
#define FLV_MUXER_H_ (1)

#include <stdbool.h>
#include <stdint.h>

void write_flv_header(bool is_have_audio, bool is_have_video);

void write_aac_sequence_header_tag(int sample_rate, int channel);

void write_avc_sequence_header_tag(const uint8_t *sps, int sps_len, const uint8_t *pps, int pps_len);

void write_aac_data_tag(const uint8_t *data, int data_len, unsigned int timestamp);

void write_avc_data_tag(const uint8_t *data, int data_len, unsigned int timestamp, int is_keyframe);

void write_audio_data_tag(const uint8_t *data, int data_len, unsigned int timestamp);

void write_video_data_tag(const uint8_t *data, int data_len, unsigned int timestamp);

#endif // FLV_MUXER_H_
