# FFmpegDemo
A video and audio editor library based on ffmpeg 6.0 api;
It can do media cut, media concat, media convert, audio mix, recording.

This project is built on visual studio 2022, and the aveditor project is a static library.
This project is base on FFmpeg 6.0.

There are 5 stage while editing a media file, (demultiplex, decode, filter, encode, multiplex).
They are running in their own thread.

In file FFmpegDemo\main.cpp, it shows demos.

But the subtitle stream is not test.

download ffmpeg 6.0:
	https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z