require "ffmpeg_composer"
require "fileutils"
require "benchmark"

file = File.expand_path("fixtures/result.mpg", File.dirname(__FILE__))
frame1 = File.expand_path("fixtures/img0.jpg", File.dirname(__FILE__))

bm = Benchmark.measure do
  1_000.times do
    FileUtils.rm(file) if File.exist?(file)
    composer = FFmpegComposer.new(file, 25)
    composer.start
    composer.add_frame(frame1, 10)
    composer.finish
    print "."
  end
end
puts "\ndone"
puts bm

sleep
