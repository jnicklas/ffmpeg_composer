#include <ruby.h>

#include <libavutil/imgutils.h>

#include "dbg.h"
#include "frames.h"

static VALUE c_ffmpeg_composer;
static VALUE c_ffmpeg_composer_error;
static VALUE c_ffmpeg_composer_context;

static VALUE ffmpeg_composer_start(VALUE self) {
  VALUE obj;
  const char* destination_path = RSTRING_PTR(rb_iv_get(self, "@path"));
  int width = NUM2INT(rb_iv_get(self, "@width"));
  int height = NUM2INT(rb_iv_get(self, "@height"));
  int fps = NUM2INT(rb_iv_get(self, "@fps"));
  struct FFCFrameContext *frame_context = ffc_alloc_frame_context(width, height, fps, destination_path);

  obj = Data_Wrap_Struct(c_ffmpeg_composer_context, 0, ffc_free_frame_context, frame_context);

  rb_iv_set(self, "@context", obj);

  return Qnil;
}

static VALUE ffmpeg_composer_finish(VALUE self) {
  int res, error=1;

  struct FFCFrameContext *frame_context=NULL;
  Data_Get_Struct(rb_iv_get(self, "@context"), struct FFCFrameContext, frame_context);
  check(frame_context != NULL, "unabled to retrieve frame context, maybe call #start?");

  res = write_delayed_frames_to_file(frame_context->file, frame_context->frame, frame_context->codec_context, &frame_context->pkt);
  check(res >= 0, "failed to write delayed frames");

  error = 0;
error:
  rb_iv_set(self, "@context", Qnil);
  if(frame_context) { ffc_close_frame_context(frame_context); }
  if(error) { rb_raise(c_ffmpeg_composer_error, "unable to finish composition"); }
  return Qnil;
}

static VALUE ffmpeg_composer_add_frame(VALUE self, VALUE source_path, VALUE duration) {
  int res;

  struct FFCFrameContext *frame_context=NULL;
  Data_Get_Struct(rb_iv_get(self, "@context"), struct FFCFrameContext, frame_context);
  check(frame_context != NULL, "unabled to retrieve frame context, maybe call #start?");

  res = write_image_to_file(frame_context->file, RSTRING_PTR(source_path), NUM2INT(duration), frame_context->frame, frame_context->codec_context, &frame_context->pkt);
  check(res >= 0, "failed to write image to file");

  return Qnil;
error:
  rb_raise(c_ffmpeg_composer_error, "unable to add frame");
  return Qnil;
}

void Init_ffmpeg_composer(void) {
  c_ffmpeg_composer = rb_define_class("FFmpegComposer", rb_cObject);
  c_ffmpeg_composer_error = rb_define_class_under(c_ffmpeg_composer, "Error", rb_eStandardError);
  c_ffmpeg_composer_context = rb_define_class_under(c_ffmpeg_composer, "Context", rb_eStandardError);

  rb_define_method(c_ffmpeg_composer, "start", ffmpeg_composer_start, 0);
  rb_define_method(c_ffmpeg_composer, "finish", ffmpeg_composer_finish, 0);
  rb_define_method(c_ffmpeg_composer, "add_frame", ffmpeg_composer_add_frame, 2);
}

