#ifndef __frames_h__
#define __frames_h__

#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>

struct FFCFrameContext {
  FILE *file;
  AVCodecContext *codec_context;
  AVFrame *frame;
  AVPacket pkt;
};

struct FFCFrameContext *ffc_alloc_frame_context();
void ffc_free_frame_context(struct FFCFrameContext *frame_context);
void ffc_close_frame_context(struct FFCFrameContext *frame_context);

AVCodecContext *get_codec_context(int width, int height, int fps);

AVFrame *get_av_frame(AVCodecContext *codec_context);

int write_delayed_frames_to_file(FILE *file, AVFrame *frame, AVCodecContext *codec_context, AVPacket *pkt);

int write_image_to_file(FILE *file, const char *filename, int count, AVFrame *frame, AVCodecContext *codec_context, AVPacket *pkt);

#endif
