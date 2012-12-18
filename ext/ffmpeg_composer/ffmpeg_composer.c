#include <ruby.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "dbg.h"

static ID id_read;
static ID id_new;
static VALUE c_ffmpeg_composer;
static VALUE c_ffmpeg_composer_error;

static VALUE ffmpeg_composer_start(VALUE self) {
  return Qnil;
}

static VALUE ffmpeg_composer_finish(VALUE self) {
  return Qnil;
error:
  rb_raise(c_ffmpeg_composer_error, "unable to finish composition");
  return Qnil;
}

static VALUE ffmpeg_composer_add_frame(VALUE self, VALUE frame, VALUE duration) {
//  VALUE content = rb_funcall(frame, id_read, 0);
//  VALUE str = StringValue(content);
  int res, i;
  res = load_image_into_frame(frame, filename);
  check(res >= 0, "failed to load image into frame");

  for (i = 0; i < count; i++) {
    res = write_frame_to_file(file, frame, codec_context, pkt);
    check(res >= 0, "unable to write frame to file");
  }

  return Qnil;
error:
  rb_raise(c_ffmpeg_composer_error, "unable to add frame");
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
