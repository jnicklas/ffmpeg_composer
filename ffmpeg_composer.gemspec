# -*- encoding: utf-8 -*-
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'ffmpeg_composer/version'

Gem::Specification.new do |gem|
  gem.name          = "ffmpeg_composer"
  gem.version       = FFmpegComposer::VERSION
  gem.authors       = ["Jonas Nicklas"]
  gem.email         = ["jonas.nicklas@gmail.com"]
  gem.description   = %q{Compose multiple images and audio files into a movie via FFmpeg}
  gem.summary       = %q{FFmpeg movie composer}
  gem.homepage      = ""

  gem.files         = `git ls-files`.split($/)
  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.extensions    = ["ext/ffmpeg_composer/extconf.rb"]
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.require_paths = ["lib"]
end
