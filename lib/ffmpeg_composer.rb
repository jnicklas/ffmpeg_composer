require "ffmpeg_composer/ffmpeg_composer"
require "ffmpeg_composer/version"

class FFmpegComposer
  def initialize(path, fps)
    @path = path
    @fps = fps
    @width = 320
    @height = 240
  end
end
