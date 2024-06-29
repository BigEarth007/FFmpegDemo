#include <iostream>
#include <string>
#include "../AVEditor/AVEditor.h"

using namespace std;
using namespace aveditor;

#pragma comment(lib, "AVEditor.lib")

/*
* Convert .mp4 to .mov 
*/
static void Cover()
{
	try
	{
		CEditor Editor;
		 
		FFormatContext& Input = Editor.OpenInputFile("4.mp4");		 
		FFormatContext& Output = Editor.AllocOutputFile("1.ts");

#if 1
		// Set Codecs and streams for output file based on input file
		Output.BuildAllStreams(Input);
#else
		// or you can do like this
		auto mInput = Input.GetCodecContext();
		Output.BuildEncodeCodecContext(AVCodecID::AV_CODEC_ID_MPEG4, 
			mInput->at(EStreamType::EST_Video).m_Context);
		Output.BuildEncodeCodecContext(AVCodecID::AV_CODEC_ID_MP3,
			mInput->at(EStreamType::EST_Audio).m_Context);

		auto mOutput = Output.GetCodecContext();
		// Modify the parameters of the codec as you need, eg:
		mOutput->at(EStreamType::EST_Video).m_Context->bit_rate = 260000;
		//mOutput->at(EStreamType::EST_Video).m_Context->framerate = { 50, 1 };
		//mOutput->at(EStreamType::EST_Video).m_Context->time_base = { 1, 12800 };
		
		Output.BuildStream(mOutput->at(EStreamType::EST_Video));
		Output.BuildStream(mOutput->at(EStreamType::EST_Audio));
#endif
		Output.GetCodecContext(EStreamType::ST_Audio)->bit_rate = 384000;
		// Split the input context into fragments
		// It can be used to cut input file
		Editor.AddSelectedSection(5, 5);
		Editor.AddSelectedSection(13, 6);
		//Editor.AddSelectedSection(22, 5);

		Editor.OpenOutputFile();

		Editor.Start();
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Append one video to another one
static void Concat()
{
	try
	{
		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile("4.mp4");
		FFormatContext& Input2 = Editor.OpenInputFile("2.mp4");

		FFormatContext& Output = Editor.AllocOutputFile("3.mp4");

		Output.BuildAllStreams(Input2);
		Output.GetCodecContext(EStreamType::ST_Video)->bit_rate = 2000000;
		Editor.OpenOutputFile();

		Editor.Start();
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Detach audio stream from input context
static void DetachAudioStream()
{
	try
	{
		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile("1.mp4", ETask::T_Normal, kStreamAudio);
		FFormatContext& Output = Editor.AllocOutputFile("1.mp3");

		Output.BuildAllStreams(Input, kStreamAudio);
		Editor.OpenOutputFile();

		Editor.Start();
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Mix audio 2.aac into 1.mp4
// it can also mix audio file
static void MixAudio()
{
	try
	{
		CEditor Editor;

		FFormatContext& Input = Editor.OpenInputFile("4.mp4", ETask::T_AMixMain);
		FFormatContext& Input2 = Editor.OpenInputFile("1.aac", ETask::T_AMixBranch);

		FFormatContext& Output = Editor.AllocOutputFile("2.avi");

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();

		Editor.Start();
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Play video, just for test, do not be care
// You can inherit IPlayer class to define your player
class OutputPlayer : public IAVIOHandle
{
public:
	int ReceiveData(const EStreamType n_eStreamType,
		void* n_Data, EDataType n_eType, int n_nIndex)
	{
		if (!n_Data) return 0;
		// display frame
		// ...
		AVDebug("Output frame %d, Type %d\n", n_eStreamType, n_eType);
		AVFreeData(n_eType, n_Data);

		return 0;
	}
};

static void Play()
{
	try
	{
		CEditor Editor;
		OutputPlayer op;

		FFormatContext& Input = Editor.OpenInputFile("1.mp4");

		AVCodecContext* vCodec = Input.GetCodecContext(EStreamType::ST_Video);
		//AVCodecContext* aCodec = Input.GetCodecContext(EStreamType::EST_Audio);

		// Maybe AVFrame decoded from input context should be convert to target 
		// sample format/pixel format, so the target codec context should be created
		FFormatContext& Output = Editor.GetOutputContext()->GetContext();
		Output.BuildEncodeCodecContext(Input.FindStream(AVMEDIA_TYPE_VIDEO));
		Output.BuildEncodeCodecContext(Input.FindStream(AVMEDIA_TYPE_AUDIO));
		//Output.BuildCodecContext(aCodec->codec_id, aCodec);

		AVCodecContext* ovCodec = Output.GetCodecContext(EStreamType::ST_Video);

		ovCodec->width = 800;
		ovCodec->height = 600;
		ovCodec->pix_fmt = GetSupportedPixelFormat(ovCodec->codec, AVPixelFormat::AV_PIX_FMT_YUV420P);

		AVCodecContext* aoCodec = Output.GetCodecContext(EStreamType::ST_Audio);
		aoCodec->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S32;

		// Frames will be sent to op
		Editor.SetOutputIOHandle(&op);

		Editor.Start();
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Recording audio, (in this way, you can also recording video)
static void RecordAudio()
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

		FFormatContext& Input = Editor.OpenInputFile(sUtf8, ETask::T_Normal, 
			kStreamAll, Device.m_InputFormat);
		FFormatContext& Output = Editor.AllocOutputFile("record.aac");

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();

		Editor.Start();

		std::thread t([&Editor]() {

			// Record 5 seconds
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			Editor.Stop();

			std::cout << "record over" << std::endl;

			});
		t.detach();
		
		std::cout << "record start" << std::endl;
		
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

// Recording PCM date to output file
class InputCtx : public IContextHandle
{
public:
	// Fill video frame when write PCM data
	virtual void FillVideoFrame(AVFrame* n_Frame,
		const void* n_Data, const int& n_nSize) const
	{
		IContextHandle::FillVideoFrame(n_Frame, n_Data, n_nSize);

		int nPlanes = GetVideoPlanes();

		if (1 < nPlanes)
		{
			for (int i = 0; i < n_Frame->height; i++)
			{
				for (int j = 0; j < n_Frame->width; j++)
				{
					// set the frame date according to source data format
					// ....
				}
			}
		}
	}

	// Fill audio frame when write PCM data
	virtual void FillAudioFrame(AVFrame* n_Frame,
		const void* n_Data, const int& n_nSize) const
	{
		IContextHandle::FillAudioFrame(n_Frame, n_Data, n_nSize);

		int nIsPlanar = GetAudioPlanar();
		int nBytesPerSample = GetBytesPerSample();

		if (0 != nIsPlanar)
		{
			uint8_t* ptr = (uint8_t*)n_Data;
			for (int i = 0; i < n_Frame->nb_samples; i++)
			{
				for (int j = 0; j < GetChannelCount(); j++)
				{
					memcpy_s(n_Frame->data[j] + i * nBytesPerSample, nBytesPerSample,
						ptr, nBytesPerSample);
					ptr += nBytesPerSample;
				}
			}
		}
	}
};
static void RecordPCM()
{
	try
	{
		CEditor Editor;
		InputCtx ctx;

		// Create an empty input context
		FFormatContext& Input = Editor.OpenInputFile("", ETask::T_Normal, kStreamVA);
		FFormatContext& Output = Editor.AllocOutputFile("output.mp4");

		// Build decode codec for input context
		// Maybe the Pixel Format/ Sample Format of the source data 
		// is not the same as output context
		// So the decode codec is need to build a converter 
		// The decode codec is built base on source data
		// The following is example.

		// Video Codec Context
		AVCodecContext* InputVideoCodec = Input.BuildDecodeCodecContext(
			EStreamType::ST_Video, AVCodecID::AV_CODEC_ID_RAWVIDEO)->m_Context;
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
			EStreamType::ST_Audio, AVCodecID::AV_CODEC_ID_FIRST_AUDIO)->m_Context;
		InputAudioCodec->bit_rate = 192000;
		InputAudioCodec->sample_rate = GetSupportedSampleRate(InputAudioCodec->codec, 41000);
		InputAudioCodec->time_base = { 1, InputAudioCodec->sample_rate };
		InputAudioCodec->sample_fmt = GetSupportedSampleFormat(InputAudioCodec->codec, AVSampleFormat::AV_SAMPLE_FMT_S16);
		GetSupportedChannelLayout(InputAudioCodec->codec, &InputAudioCodec->ch_layout);

		Output.BuildAllStreams(Input);
		Editor.OpenOutputFile();

		Editor.SetInputContextHandle(&ctx, 0);

		// the you can call following function to write frame data
		while (true)
		{
			//Editor.WriteFrameDatas(EStreamType::EST_Video, a, b, 0);
			//Editor.WriteFrameDatas(EStreamType::EST_Audio, a, b, 0);
		}

		Editor.Start();
		Editor.Join();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
}

int main()
{
	auto start = std::chrono::steady_clock::now();
	//SetupEditorLog();
	
	//Cover();
	Concat();
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
