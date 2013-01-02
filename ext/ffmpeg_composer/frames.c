#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "dbg.h"
#include "frames.h"

static int sws_flags = SWS_BICUBIC;

int load_image_into_frame(AVFrame *frame, const char *filename)
{
  int retval = -1, res, frame_decoded;

  AVInputFormat *format = NULL;
  AVFormatContext *format_ctx = NULL;
  AVCodec *codec=NULL;
  AVCodecContext *codec_ctx=NULL;
  AVFrame *source_frame=NULL;
  AVPacket pkt;
  static struct SwsContext *sws_ctx;

  av_register_all();

  format = av_find_input_format("image2");

  res = avformat_open_input(&format_ctx, filename, format, NULL);
  check(res >= 0, "Failed to open input file '%s'", filename);

  codec_ctx = format_ctx->streams[0]->codec;
  codec = avcodec_find_decoder(codec_ctx->codec_id);
  check(codec, "Failed to find codec");

  res = avcodec_open2(codec_ctx, codec, NULL);
  check(res >= 0, "Failed to open codec");

  source_frame = avcodec_alloc_frame();
  check(source_frame, "Failed to alloc source frame");

  res = av_read_frame(format_ctx, &pkt);
  check(res >= 0, "Failed to read frame from file");

  res = avcodec_decode_video2(codec_ctx, source_frame, &frame_decoded, &pkt);
  check(res >= 0 && frame_decoded, "Failed to decode image from file");

  sws_ctx = sws_getContext(source_frame->width, source_frame->height, source_frame->format,
      frame->width, frame->height, frame->format,
      sws_flags, NULL, NULL, NULL);
  check(sws_ctx, "unable to initialize scaling context");

  sws_scale(sws_ctx,
      (const uint8_t * const *)source_frame->data, source_frame->linesize,
      0, frame->height, frame->data, frame->linesize);

  retval = 0;
error:
  if(codec_ctx) { avcodec_close(codec_ctx); }
  if(format_ctx) { avformat_close_input(&format_ctx); }
  if(source_frame) { av_freep(&source_frame); }
  if(sws_ctx) { sws_freeContext(sws_ctx); }
  av_free_packet(&pkt);
  return retval;
}

int write_frame_to_file(FILE *file, AVFrame *frame, AVCodecContext *codec_context, AVPacket *pkt) {
  int res, got_output;
  av_init_packet(pkt);
  pkt->data = NULL;
  pkt->size = 0;

  /* generate synthetic video */
  frame->pts += 1;

  res = avcodec_encode_video2(codec_context, pkt, frame, &got_output);
  check(res >= 0, "Error encoding frame");

  if (got_output) {
    fwrite(pkt->data, 1, pkt->size, file);
    av_free_packet(pkt);
  }
  return 0;
error:
  return -1;
}

int write_image_to_file(FILE *file, const char *filename, int count, AVFrame *frame, AVCodecContext *codec_context, AVPacket *pkt) {
  int res, i;
  res = load_image_into_frame(frame, filename);
  check(res >= 0, "failed to load image into frame");

  for (i = 0; i < count; i++) {
    res = write_frame_to_file(file, frame, codec_context, pkt);
    check(res >= 0, "unable to write frame to file");
  }

  return 0;
error:
  return -1;
}

int write_delayed_frames_to_file(FILE *file, AVFrame *frame, AVCodecContext *codec_context, AVPacket *pkt) {
  int res, got_output;

  for (got_output = 1; got_output;) {
    res = avcodec_encode_video2(codec_context, pkt, NULL, &got_output);
    check(res >= 0, "Error encoding frame");

    if (got_output) {
      fwrite(pkt->data, 1, pkt->size, file);
      av_free_packet(pkt);
    }
  }

  return 0;
error:
  return -1;
}

AVCodecContext *get_codec_context(int width, int height, int fps)
{
  int res;
  AVCodec *codec;
  AVCodecContext *codec_context = NULL;

  avcodec_register_all();

  codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
  check(codec, "unable to find codec");

  codec_context = avcodec_alloc_context3(codec);
  check(codec_context, "unable to allocate codec");

  codec_context->bit_rate = 400000;
  codec_context->width = width;
  codec_context->height = height;
  codec_context->time_base= (AVRational){1,fps};
  codec_context->gop_size = 10;
  codec_context->max_b_frames=1;
  codec_context->pix_fmt = PIX_FMT_YUV420P;

  res = avcodec_open2(codec_context, codec, NULL);
  check(res >= 0, "could not open codec");

  return codec_context;
error:
  return NULL;
}

AVFrame *get_av_frame(AVCodecContext *codec_context) {
  int res;
  AVFrame *frame;

  frame = avcodec_alloc_frame();
  check(frame != NULL, "unable to allocate frame");
  frame->height = codec_context->height;
  frame->width = codec_context->width;
  frame->format = codec_context->pix_fmt;
  frame->pts = 0;

  res = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, frame->format, 1);
  check(res >= 0, "failed to allocate memory for video frame");

  return frame;
error:
  return NULL;
}
