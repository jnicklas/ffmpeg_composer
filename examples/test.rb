require "ffmpeg_composer"

file = File.open File.expand_path("fixtures/result.mpg", File.dirname(__FILE__)), "wb"

frame1 = File.open File.expand_path("fixtures/img0.jpg", File.dirname(__FILE__)), "r"
frame2 = File.open File.expand_path("fixtures/img1.jpg", File.dirname(__FILE__)), "r"
frame3 = File.open File.expand_path("fixtures/img2.jpg", File.dirname(__FILE__)), "r"
frame4 = File.open File.expand_path("fixtures/img3.jpg", File.dirname(__FILE__)), "r"

composer = FFmpegComposer.new(file)
composer.start
composer.add_frame(frame1, 10)
composer.add_frame(frame2, 20)
composer.add_frame(frame3, 50)
composer.add_frame(frame4, 10)
composer.finish
