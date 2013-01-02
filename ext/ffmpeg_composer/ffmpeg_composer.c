#include <ruby.h>

#include <libavutil/imgutils.h>

#include "dbg.h"
#include "frames.h"

static ID id_read;
static ID id_new;
static VALUE c_ffmpeg_composer;
static VALUE c_ffmpeg_composer_error;

static VALUE ffmpeg_composer_start(VALUE self) {
  return Qnil;
}

static VALUE ffmpeg_composer_finish(VALUE self) {
  int res, error=1;
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

  error = 0;
error:
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
  int res, error=1;
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

  error = 0;
error:
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
