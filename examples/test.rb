require "ffmpeg_composer"
require "fileutils"

file = File.expand_path("fixtures/result.mpg", File.dirname(__FILE__))
FileUtils.rm(file) if File.exist?(file)

frame1 = File.expand_path("fixtures/img0.jpg", File.dirname(__FILE__))
frame2 = File.expand_path("fixtures/img1.jpg", File.dirname(__FILE__))
frame3 = File.expand_path("fixtures/img2.jpg", File.dirname(__FILE__))
frame4 = File.expand_path("fixtures/img3.jpg", File.dirname(__FILE__))

composer = FFmpegComposer.new(file, 25)
composer.add_frame(frame1, 10)
composer.add_frame(frame2, 20)
composer.add_frame(frame3, 50)
composer.add_frame(frame4, 10)
composer.finish

`open #{file}`
