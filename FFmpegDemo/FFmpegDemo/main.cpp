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

#if 1
		// Set Codecs and streams for output file based on input file
		Output.BuildAllStreams(Input);
#else
		// or you can do like this
		auto mInput = Input.GetCodecContext();
		Output.BuildEncodeCodecContext(AVcodecID::AV_CODEC_ID_MPEG4, 
			mInput->at(EStreamType::EST_Video).m_Context);
		Output.BuildEncodeCodecContext(AVcodecID::AV_CODEC_ID_MP3, 
			mInput->at(EStreamType::EST_Audio).m_Context);

		auto mOutput = Output.GetCodecContext();
		// Modify the parameters of the codec as you need, eg:
		mOutput->at(EStreamType::EST_Video).m_Context->framerate = { 50, 1 };
		mOutput->at(EStreamType::EST_Video).m_Context->time_base = { 1, 12800 };
		
		Output.BuildStream(mOutput->at(EStreamType::EST_Video));
		Output.BuildStream(mOutput->at(EStreamType::EST_Audio));
#endif
		
		Editor.OpenOutputFile();
		 
		Editor.CreateAllStage();
		//CDemuxer* Demuxer = (CDemuxer*)Editor.GetCache().GetContextInfo(0)->Demuxer;
		// We just need one section of the input context, it can be used for video cut
		//Demuxer->SelectSection(70, 330);

		Editor.StartEdit();
		Editor.JoinEdit();
		// Or you can start a thread to edit video
		// Editor.Start();
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

		FFormatContext& Output = Editor.AllocOutputFile("3.mp4");

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();
		Editor.CreateAllStage();

		Editor.StartEdit();
		Editor.JoinEdit();
		// Or you can start a thread to edit video
		// Editor.Start();
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

		FFormatContext& Input = Editor.OpenInputFile("1.mp4", EJob::EJ_Normal, kStreamAudio);
		FFormatContext& Output = Editor.AllocOutputFile("1.mp3");

		Output.BuildAllStreams(Input, kStreamAudio);
		Editor.OpenOutputFile();
		Editor.CreateAllStage();

		Editor.StartEdit();
		Editor.JoinEdit();
		// Or you can start a thread to edit video
		// Editor.Start();
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

		FFormatContext& Input = Editor.OpenInputFile("2.mp4", EJob::EJ_AMixMain);
		FFormatContext& Input2 = Editor.OpenInputFile("1.mp3", EJob::EJ_AMixBranch);

		FFormatContext& Output = Editor.AllocOutputFile("2.mov");

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();
		Editor.CreateAllStage();

		Editor.StartEdit();
		Editor.JoinEdit();
		// Or you can start a thread to edit video
		// Editor.Start();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Play video, just for test, do not be care
// You can inherit IPlayer class to define your player
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
		Output.BuildEncodeCodecContext(Input.FindStream(AVMEDIA_TYPE_VIDEO));
		Output.BuildEncodeCodecContext(Input.FindStream(AVMEDIA_TYPE_AUDIO));
		// could not use this function, as the frame_size is 0
		//Output.BuildCodecContext(aCodec->codec_id, aCodec);

		AVCodecContext* aoCodec = Output.GetCodecContext(EStreamType::EST_Audio);
		aoCodec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S32;

		// If parameter is nullptr(default), it will instance CSDLPlayer object
		CSDLPlayer* Player = (CSDLPlayer*)Editor.CreatePlayer();
		Player->InitAudio(aoCodec);

		Player->SetStartupCallback([&Player, &vCodec]() {
			Player->InitVideo("sdl player", vCodec->width, vCodec->height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
			}
		);

		Player->SetFinishedCallback([&Editor]() {
			Editor.StopEdit();
			Editor.GetCache().Release();
			}
		);

		Editor.StartEdit();
		Editor.JoinEdit();
		// Or you can start a thread to edit video
		// Editor.Start();
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
		Editor.CreateAllStage();

		Editor.StartEdit();

		std::thread t([=]() {

			// Record 5 seconds
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			CDemuxer* Demuxer = (CDemuxer*)Editor.GetCache().GetContextInfo(0)->Demuxer;
			Demuxer->Stop();

			std::cout << "record over" << std::endl;

			});
		t.detach();
		
		std::cout << "record start" << std::endl;
		
		Editor.JoinEdit();
		// Or you can start a thread to edit video
		// Editor.Start();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Recording PCM date to output file
void RecordPCM()
{
	try
	{
		CEditor Editor;

		// Create an empty input context
		FFormatContext& Input = Editor.OpenInputFile("", EJob::EJ_Normal, kStreamVA);
		FFormatContext& Output = Editor.AllocOutputFile("1.mp4");

		// Build decode codec for input context
		// Maybe the Pixel Format/ Sample Format of the source data 
		// is not the same as output context
		// So the decode codec is need to build a converter 
		// The decode codec is built base on source data
		// The following is example.

		// Video Codec Context
		AVCodecContext* InputVideoCodec = Input.BuildDecodeCodecContext(
			EStreamType::EST_Video, AVCodecID::AV_CODEC_ID_RAWVIDEO)->m_Context;
		InputVideoCodec->width = 640;
		InputVideoCodec->height = 480;
		InputVideoCodec->bit_rate = 128000;
		// assume that frame rate of source data is 30
		InputVideoCodec->time_base = GetSupportedFrameRate(InputVideoCodec->codec, { 1, 30 });
		InputVideoCodec->framerate = av_inv_q(InputVideoCodec->time_base);
		// assume that pixel format of source data is AV_PIX_FMT_YUV420P
		InputVideoCodec->pix_fmt = GetSupportedPixelFormat(InputVideoCodec->codec, AVPixelFormat::AV_PIX_FMT_YUV420P);

		// Audio Codec Context
		AVCodecContext* InputAudioCodec = Input.BuildDecodeCodecContext(
			EStreamType::EST_Audio, AVCodecID::AV_CODEC_ID_FIRST_AUDIO)->m_Context;
		InputAudioCodec->bit_rate = 192000;
		InputAudioCodec->sample_rate = GetSupportedSampleRate(InputAudioCodec->codec, 41000);
		InputAudioCodec->time_base = { 1, InputAudioCodec->sample_rate };
		InputAudioCodec->sample_fmt = GetSupportedSampleFormat(InputAudioCodec->codec, AVSampleFormat::AV_SAMPLE_FMT_S16);
		GetSupportedChannelLayout(InputAudioCodec->codec, &InputAudioCodec->ch_layout);

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();

		Editor.CreateAllStage();

		// Set callback function to parse PCM data
		Editor.SetCallbackFillVideoFrame(
			[=](AVFrame* n_Frame, const void* n_Data, const int& n_nSize, 0) {

				int nPlanes = GetPixFmtPlaneCount(InputVideoCodec->pix_fmt);

				if (1 == nPlanes)
				{
					memcpy_s(n_Frame->data[0], n_Frame->linesize[0],
						n_Data, n_nSize);
				}
				else
				{
					for (int i = 0; i < InputVideoCodec->height; i++)
					{
						for (int j = 0; j < InputVideoCodec->width; j++)
						{
							// set the frame date according to source data format
							// ....
						}
					}
				}
			}
		);
		Editor.SetCallbackFillAudioFrame(
			[=](AVFrame* n_Frame, const void* n_Data, const int& n_nSize, 0) {

				int nIsPlanar = IsSampleFmtPlanar(InputAudioCodec->sample_fmt);
				int nBytesPerSample = GetBytesPerSample(InputAudioCodec->sample_fmt);

				if (0 == nIsPlanar)
				{
					memcpy_s(n_Frame->data[0], n_Frame->linesize[0],
						n_Data, n_nSize);
				}
				else
				{
					uint8_t* ptr = (uint8_t*)n_Data;
					for (int i = 0;i < n_Frame->nb_samples; i++)
					{
						for (int j = 0;j < InputAudioCodec->ch_layout.nb_channels; j++)
						{
							memcpy_s(n_Frame->data[j] + i * nBytesPerSample, nBytesPerSample,
								ptr, nBytesPerSample);
							ptr += nBytesPerSample;
						}
					}
				}
			}
		);

		// the you can call following function to write frame data
		while (true)
		{
			//Editor.WriteFrameDatas(EStreamType::EST_Video, a, b, 0);
			//Editor.WriteFrameDatas(EStreamType::EST_Audio, a, b, 0);
		}


		Editor.StartEdit();
		Editor.JoinEdit();
		// Or you can start a thread to edit video
		// Editor.Start();
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
	
	//Cover();
	//Concat();
	//DetachAudioStream();
	//MixAudio();
	//Play();
	//RecordAudio();
	//RecordPCM();

	auto end = std::chrono::steady_clock::now();
	auto tt = end - start;

	cout << "Time cost: " << tt.count() * 1.0f / 1000 / 1000 << " millisecond." << endl;

	return 0;
}
