require "ffmpeg_composer/ffmpeg_composer"
require "ffmpeg_composer/version"

class FFmpegComposer
  def initialize(path, options={})
    @path = path
    @fps = options.fetch(:fps)
    @width = options.fetch(:width)
    @height = options.fetch(:height)
  end
end
