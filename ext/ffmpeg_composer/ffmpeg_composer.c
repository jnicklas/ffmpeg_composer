#include <ruby.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>

#include "dbg.h"
#include "lavfutils.h"

static int sws_flags = SWS_BICUBIC;
static ID id_read;
static ID id_new;
static VALUE c_ffmpeg_composer;
static VALUE c_ffmpeg_composer_error;

int load_image_into_frame(AVFrame *frame, const char *filename)
{
  int retval = -1, res;
  static struct SwsContext *sws_ctx;
  uint8_t *image_data[4];
  int linesize[4];
  int source_width, source_height;
  enum PixelFormat source_fmt;

  res = ff_load_image(image_data, linesize, &source_width, &source_height, &source_fmt, filename, NULL);
  check(res >= 0, "failed to load image");

  if (source_fmt != frame->format) {
    sws_ctx = sws_getContext(source_width, source_height, source_fmt,
        frame->width, frame->height, frame->format,
        sws_flags, NULL, NULL, NULL);
    check(sws_ctx, "unable to initialize scaling context");

    sws_scale(sws_ctx,
        (const uint8_t * const *)image_data, linesize,
        0, frame->height, frame->data, frame->linesize);
  }

  retval = 0;
error:
  if (image_data) {
    printf("freeing data");
    av_freep(&image_data[0]);
  }
  if (sws_ctx) {
    av_free(sws_ctx);
  }
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

AVCodecContext *get_codec_context(width, height, fps)
{
  int res;
  avcodec_register_all();

  AVCodec *codec;
  AVCodecContext *codec_context = NULL;

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

static VALUE ffmpeg_composer_start(VALUE self) {
  return Qnil;
}

static VALUE ffmpeg_composer_finish(VALUE self) {
  int res, error=0;
  size_t res2;
  FILE *file=NULL;
  AVCodecContext *codec_context=NULL;
  AVFrame *frame=NULL;
  AVPacket pkt;
  uint8_t endcode[] = { 0, 0, 1, 0xb7 };
  VALUE destination_path = rb_iv_get(self, "@path");
  VALUE fps = rb_iv_get(self, "@fps");

  codec_context = get_codec_context(320, 240, NUM2INT(fps));
  check(codec_context != NULL, "unable to obtain encoding context");

  file = fopen(RSTRING_PTR(destination_path), "ab");
  check(file != NULL, "could not open destination file %s", RSTRING_PTR(destination_path));

  frame = get_av_frame(codec_context);
  check(frame != NULL, "unable to allocate frame");

  res = write_delayed_frames_to_file(file, frame, codec_context, &pkt);
  check(res >= 0, "failed to write delayed frames");

  res2 = fwrite(endcode, 1, sizeof(endcode), file);
  check(res2 == sizeof(endcode), "failed to write endcode");

  goto clean;
error:
  error = 1;
clean:
  if (file) {
    fclose(file);
  }
  if (codec_context) {
    avcodec_close(codec_context);
    av_free(codec_context);
  }
  if (frame) {
    av_freep(&frame->data[0]);
    av_free(frame);
  }
  if(error) { rb_raise(c_ffmpeg_composer_error, "unable to finish composition"); }
  return Qnil;
}

static VALUE ffmpeg_composer_add_frame(VALUE self, VALUE source_path, VALUE duration) {
  int res, error=0;
  FILE *file=NULL;
  AVCodecContext *codec_context=NULL;
  AVFrame *frame=NULL;
  AVPacket pkt;
  VALUE destination_path = rb_iv_get(self, "@path");

  codec_context = get_codec_context(320, 240, 25);
  check(codec_context != NULL, "unable to obtain encoding context");

  file = fopen(RSTRING_PTR(destination_path), "ab");
  check(file != NULL, "could not open destination file %s", RSTRING_PTR(destination_path));

  frame = get_av_frame(codec_context);
  check(frame != NULL, "unable to allocate frame");

  res = write_image_to_file(file, RSTRING_PTR(source_path), NUM2INT(duration), frame, codec_context, &pkt);
  check(res >= 0, "failed to write image to file");

  goto clean;
error:
  error = 1;
clean:
  if (file) {
    fclose(file);
  }
  if (codec_context) {
    avcodec_close(codec_context);
    av_free(codec_context);
  }
  if (frame) {
    av_freep(&frame->data[0]);
    av_free(frame);
  }
  if(error) { rb_raise(c_ffmpeg_composer_error, "unable to add frame"); }
  return Qnil;
}

void Init_ffmpeg_composer(void) {
  c_ffmpeg_composer = rb_define_class("FFmpegComposer", rb_cObject);
  c_ffmpeg_composer_error = rb_define_class_under(c_ffmpeg_composer, "Error", rb_eStandardError);

  rb_define_method(c_ffmpeg_composer, "start", ffmpeg_composer_start, 0);
  rb_define_method(c_ffmpeg_composer, "finish", ffmpeg_composer_finish, 0);
  rb_define_method(c_ffmpeg_composer, "add_frame", ffmpeg_composer_add_frame, 2);
  id_read = rb_intern("read");
  id_new = rb_intern("new");
}
