#include <ruby.h>

/* our new native method; it just returns
 * the string "bonjour!" */
static VALUE hello_ffmpeg_composer(VALUE self) {
  return rb_str_new2("bonjour!");
}

/* ruby calls this to load the extension */
void Init_ffmpeg_composer(void) {
  VALUE klass = rb_define_class("FFmpegComposer", rb_cObject);

  rb_define_singleton_method(klass, "hello_world", hello_ffmpeg_composer, 0);
}
