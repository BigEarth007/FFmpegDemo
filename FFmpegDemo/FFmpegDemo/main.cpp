#include <iostream>
#include <string>
#include "../AVEditor/AVEditor.h"

using namespace std;
using namespace aveditor;

#pragma comment(lib, "AVEditor.lib")

/*
* Convert .mp4 to .mov 
*/
void Cover()
{
	try
	{
		CEditor Editor;
		 
		FFormatContext& Input = Editor.OpenInputFile("1.mp4");		 
		FFormatContext& Output = Editor.AllocOutputFile("1.avi");
		 
		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();
		 
		Editor.CreateDemuxer();
		CDemuxer* Demuxer = (CDemuxer*)Editor.GetCache().GetContextInfo(0)->Muxer;
		// We just need one section of the input context, it can be used for video cut
		Demuxer->SelectSection(70, 330);

		// Both of the following are ok
		// As .mp4 and .mov support the same audio and video encoding types,
		// the following step can be omitted
		// If convert .mp4 to .avi, do decoding and encoding is a good choose
#if 1
		Editor.CreateDecoder();
		Editor.CreateEncoder();
#else
		Editor.CreateTranscoder();
#endif

		Editor.CreateMuxer();
		 
		Editor.Start();
		Editor.Join();
		Editor.CloseOutputFile();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Append one video to another one
void Concat()
{
	try
	{
		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile("1.mp4");
		FFormatContext& Input2 = Editor.OpenInputFile("2.mp4");
		//auto m = Input.GetDecoderCodecContext();

		FFormatContext& Output = Editor.AllocOutputFile("3.mp4");

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();

		Editor.CreateDemuxer();

		// Both of the following are ok
		// As input video and output video support the same audio and video 
		// encoding types, the following step can be omitted
		// If output video is .avi, do decoding and encoding is a good choose
#if 0
		Editor.CreateDecoder();
		Editor.CreateEncoder();
#else
		Editor.CreateTranscoder();
#endif

		Editor.CreateMuxer();

		Editor.Start();
		Editor.Join();
		Editor.CloseOutputFile();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Detach audio stream from input context
void DetachAudioStream()
{
	try
	{
		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile("2.mov", EJob::EJ_Normal, kStreamAudio);

		FFormatContext& Output = Editor.AllocOutputFile("2.aac");

		Output.BuildAllStreams(Input, kStreamAudio);
		Editor.OpenOutputFile();

		Editor.CreateDemuxer();

		// Both of the following are ok
		// As input video and output video support the same audio and video 
		// encoding types, the following step can be omitted
		// If output video is .avi, do decoding and encoding is a good choose
#if 0
		Editor.CreateDecoder();
		Editor.CreateEncoder();
#else
		Editor.CreateTranscoder();
#endif

		Editor.CreateMuxer();

		Editor.Start();
		Editor.Join();
		Editor.CloseOutputFile();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Mix audio 2.aac into 1.mp4
// it can also mix audio file
void MixAudio()
{
	try
	{
		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile("1.mp4", EJob::EJ_AMixMain);
		FFormatContext& Input2 = Editor.OpenInputFile("chuanqi.mp3", EJob::EJ_AMixBranch);

		FFormatContext& Output = Editor.AllocOutputFile("2.mp4");

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();

		Editor.CreateDemuxer();

		// Both of the following are ok
		// As input video and output video support the same audio and video 
		// encoding types, the following step can be omitted
		// If output video is .avi, do decoding and encoding is a good choose

		Editor.CreateDecoder();
		Editor.CreateFilter(EStreamType::EST_Audio);
		Editor.CreateEncoder();

		Editor.CreateMuxer();

		Editor.Start();
		Editor.Join();
		Editor.CloseOutputFile();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Play video, just for test, do not be care
void Play()
{
	try
	{
		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile("tv.mp4");

		AVCodecContext* vCodec = Input.GetCodecContext(EStreamType::EST_Video);
		//AVCodecContext* aCodec = Input.GetCodecContext(EStreamType::EST_Audio);

		// Maybe AVFrame decoded from input context should be convert to target 
		// sample format/pixel format, so the target codec context should be created
		FFormatContext& Output = Editor.GetOutputContext();
		Output.BuildCodecContext(Input.FindStream(AVMEDIA_TYPE_VIDEO));
		Output.BuildCodecContext(Input.FindStream(AVMEDIA_TYPE_AUDIO));
		// could not use this function, as the frame_size is 0
		//Output.BuildCodecContext(aCodec->codec_id, aCodec);

		AVCodecContext* aoCodec = Output.GetCodecContext(EStreamType::EST_Audio);
		aoCodec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S32;

		Editor.CreateDemuxer();
		Editor.CreateDecoder();

		CPlayer* Player = Editor.CreatePlayer();
		Player->InitAudio(aoCodec);

		Player->SetStartupCallback([&Player, &vCodec]() {
			Player->InitVideo("sdl player", vCodec->width, vCodec->height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
			}
		);

		Player->SetFinishedCallback([&Editor]() {
			Editor.Stop();
			Editor.GetCache().Release();
			}
		);

		Editor.Start();
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Recording audio, (in this way, you can also recording video)
void RecordAudio()
{
	try
	{
		SetupEditorDevice();
		FDevice Device;
		Device.Alloc("dshow");

		if (!Device.m_Context) return;

		std::string sDevice = "audio=";
		sDevice +=	Device.GetDeviceDescription(0);
		std::string sUtf8 = AnsiToUtf8(sDevice);

		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile(sUtf8, EJob::EJ_Normal, 
			kStreamAll, Device.m_InputFormat);

		FFormatContext& Output = Editor.AllocOutputFile("record.aac");

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();

		Editor.CreateDemuxer();

		//Editor.CreateDecoder();
		//Editor.CreateEncoder();
		Editor.CreateTranscoder();

		Editor.CreateMuxer();

		Editor.Start();
		Editor.Join();
		Editor.CloseOutputFile();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

#undef main
int main()
{
	auto start = std::chrono::steady_clock::now();
	//SetupEditorLog();
	
	//Concat();
	//Cover();
	Play();
	//DetachAudioStream();
	//RecordAudio();
	//MixAudio();

	auto end = std::chrono::steady_clock::now();
	auto tt = end - start;

	cout << "Time cost: " << tt.count() * 1.0f / 1000 / 1000 << " millisecond." << endl;

	return 0;
}
