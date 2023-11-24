# FFmpegDemo
A video and audio editor library based on ffmpeg 6.0 api;
It can do media cut, media concat, media convert, audio mix, recording.

This project is built on visual studio 2022, and the aveditor project is a static library.
This project is base on FFmpeg 6.0, and SDL 2.28.5.

There are 4 stage while editing a media file, (demultiplex, decode, encode, multiplex).

Firstly, For every input file, i create one thread to demultiplex for each input file.
Then, decoding, filtering (used for audio mixing) and encoding every stream in Independent threads.
At last, writing all packet into output file.

In file FFmpegDemo\main.cpp, it shows demo.

But the subtitle stream is not test.

download ffmpeg 6.0:
	https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z
	
download sdl 2.28.5:
	https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-devel-2.28.5-VC.zip