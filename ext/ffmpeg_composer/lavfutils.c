/*
 * Copyright 2012 Stefano Sabatini <stefasab gmail com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <libavutil/imgutils.h>
#include "lavfutils.h"
#include "dbg.h"

int ff_load_image(uint8_t *data[4], int linesize[4],
                  int *w, int *h, enum PixelFormat *pix_fmt,
                  const char *filename, void *log_ctx)
{
    AVInputFormat *iformat = NULL;
    AVFormatContext *format_ctx = NULL;
    AVCodec *codec=NULL;
    AVCodecContext *codec_ctx=NULL;
    AVFrame *frame=NULL;
    int frame_decoded, retVal = -1, res;
    AVPacket pkt;

    av_register_all();

    iformat = av_find_input_format("image2");

    res = avformat_open_input(&format_ctx, filename, iformat, NULL);
    check(res >= 0, "Failed to open input file '%s'", filename);

    codec_ctx = format_ctx->streams[0]->codec;
    codec = avcodec_find_decoder(codec_ctx->codec_id);
    check(codec, "Failed to find codec");

    res = avcodec_open2(codec_ctx, codec, NULL);
    check(res >= 0, "Failed to open codec");

    frame = avcodec_alloc_frame();
    check(frame, "Failed to alloc frame");

    res = av_read_frame(format_ctx, &pkt);
    check(res >= 0, "Failed to read frame from file");

    res = avcodec_decode_video2(codec_ctx, frame, &frame_decoded, &pkt);
    check(res >= 0 && frame_decoded, "Failed to decode image from file");

    *w       = frame->width;
    *h       = frame->height;
    *pix_fmt = frame->format;

    res = av_image_alloc(data, linesize, *w, *h, *pix_fmt, 16);
    check(res >= 0, "failed to alloc image");

    av_image_copy(data, linesize, (const uint8_t **)frame->data, frame->linesize, *pix_fmt, *w, *h);

    retVal = 0;
error:
    if(codec_ctx) { avcodec_close(codec_ctx); }
    if(format_ctx) { avformat_close_input(&format_ctx); }
    if(frame) { av_freep(&frame); }
    av_free_packet(&pkt);

    return retVal;
}
