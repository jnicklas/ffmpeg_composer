require "ffmpeg_composer/ffmpeg_composer"
require "ffmpeg_composer/version"

class FFmpegComposer
  def initialize(path, options={})
    @path = path.to_s
    @fps = options.fetch(:fps).to_i
    @width = options.fetch(:width).to_i
    @height = options.fetch(:height).to_i
  end
end
