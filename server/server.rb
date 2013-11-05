#!/usr/bin/env ruby

require "pebble"

class OSAScript
	def initialize(app_name)
		@app_name = app_name
	end

	def evaluate(command)
		`osascript -e 'tell application "#{@app_name}" to #{command}'`
	end
end

class Model
	attr_accessor :artist, :album, :track
	def initialize(backend)
		@itunes = backend
	end

	def update(watch)
		artist = @itunes.evaluate("artist of current track as string").strip
		album = @itunes.evaluate( "album of current track as string").strip
		track = @itunes.evaluate( "name of current track as string").strip
		unless (@artist == artist && @album == album  && @track == track )
			puts "Updating nowplaying metadata"
			@artist = artist
			@album  = album
			@track  = track
			watch.set_nowplaying_metadata(@artist, @album, @track)
		else
			puts "No Updating as metadata is uncached"
		end
	end
end

class Controller
	def initialize(model)
		@watch = Pebble::Watch.autodetect
		@model = model
		@update_tid = nil
	end
	
	def start
		@watch.connect
		register_handlers
		@watch.listen_for_events
		@update_tid = Thread.new do
			loop do
				update
				sleep 5
			end
		end
		@update_tid.join
	end

	def update
		@model.update(@watch)
	end

	def register_handlers
		@watch.on_event(:media_control) do |event|
			commands = {
				playpause: "playpause",
				next: "next track",
				previous: "previous track"
			}

			puts "Executing #{event.button} command"

			@app.evaluate(commands[event.button])
			update
		end
	end
end

model = Model.new(
	OSAScript.new('iTunes')
)
controller = Controller.new(model)
controller.start
