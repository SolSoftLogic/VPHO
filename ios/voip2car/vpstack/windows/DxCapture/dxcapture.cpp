#ifndef _WIN32_WCE
#define _WIN32_WINNT 0x400
#endif
#include <stdio.h>
#include <tchar.h>

#ifdef _WIN32_WCE
#define USEDSSINK
#include <windows.h>
#include <Ocidl.h>
#include <oleauto.h>
#include "../../wince/cpropertybag.h"
#include "../../wince/dssink/dssink.h"
#endif

//typedef DWORD *DWORD_PTR;
//typedef LONG *LONG_PTR;

#include "dxcapture.h"
#ifdef USEBDA
#include "bdagraph.h"
#endif
#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

//MIDL_DEFINE_GUID(IID, CLSID_NullRenderer, 0xC1F400A4, 0x3F08, 0x11d3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37);
//MIDL_DEFINE_GUID(IID, CLSID_SampleGrabber, 0xC1F400A0, 0x3F08, 0x11d3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37);
//MIDL_DEFINE_GUID(IID, IID_ISampleGrabber, 0x6B652FFF, 0x11FE, 0x4fce, 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F);
#if !defined _WIN32_WCE || _WIN32_WCE < 0x600
MIDL_DEFINE_GUID(IID, MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
#endif
MIDL_DEFINE_GUID(IID, IID_IDecklinkIOControl, 0x60F58A81, 0xA387, 0x4922, 0xAA, 0xAC, 0x99, 0x8B, 0xD9, 0xFB, 0xE1, 0xAA);
#if _MSC_VER <= 1300
MIDL_DEFINE_GUID(IID, CLSID_VideoCapture, 0xf80b6e95, 0xb55a, 0x4619, 0xae, 0xc4, 0xa1, 0xe, 0xae, 0xde, 0x98, 0xc);
#endif
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

static void WINAPI DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if(pmt->cbFormat)
        CoTaskMemFree((PVOID)pmt->pbFormat);
    if(pmt->pUnk)
        pmt->pUnk->Release();
    CoTaskMemFree((PVOID)pmt);
}

int DXCAPTURE::OpenError(const char *error, HRESULT hr)
{
	strcpy(lasterrorstr, error);
	SAFE_RELEASE(pMC);
	SAFE_RELEASE(pNullAudioRenderer);
#ifndef _WIN32_WCE
	SAFE_RELEASE(pIAudioGrabber);
	SAFE_RELEASE(pAudioGrabber);
#endif
	SAFE_RELEASE(pNullVideoRenderer);
	SAFE_RELEASE(pDroppedFrames);
#ifdef INCLUDE_DECKLINK
	SAFE_RELEASE(pDecklinkIOControl);
	SAFE_RELEASE(pTimecodeCaptureSource);
	SAFE_RELEASE(pNullTimecodeRenderer);
	SAFE_RELEASE(pITimecodeGrabber);
	SAFE_RELEASE(pTimecodeGrabber);
#endif
	SAFE_RELEASE(pTimecodeReader);
#ifndef _WIN32_WCE
	SAFE_RELEASE(pIVideoGrabber);
	SAFE_RELEASE(pVideoGrabber);
#endif
	SAFE_RELEASE(pMjpegDec);
	SAFE_RELEASE(pTVTuner);
	SAFE_RELEASE(pCrossbar);
	SAFE_RELEASE(pGB);
	SAFE_RELEASE(pCGB);
#ifdef USEBDA
	if(bda)
	{
		delete bda;
		bda = 0;
	}
#endif
//	SAFE_RELEASE(pBuilder);
	return -1;
}

static HRESULT GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
{
	HRESULT hr = S_OK;

	if(pFilter)
	{
		IEnumPins *pEnum = NULL;
		IPin *pPin = NULL;

		hr = pFilter->EnumPins(&pEnum);

		if (FAILED(hr))
			return hr;

		while (pEnum->Next(1, &pPin, NULL) == S_OK)
		{
			PIN_DIRECTION ThisPinDir;
			pPin->QueryDirection(&ThisPinDir);
			if (ThisPinDir == PinDir)
			{
				IPin *pPinTemp = NULL;

				hr = pPin->ConnectedTo(&pPinTemp);
				if (SUCCEEDED(hr))
					pPinTemp->Release();
				else
				{
					// unconnected
					pEnum->Release();
					*ppPin = pPin;	// NOTE: Outstanding reference count!
					return S_OK;
				}
			}
			pPin->Release();
		}
		pEnum->Release();
		return E_FAIL;
	}
	return hr;
}

int DXCAPTURE::EnumFormats(VIDEOFORMAT *formats, int max)
{
	IAMStreamConfig *psc;
	HRESULT hr;
	IPin *pIPin;
	int count, i, size;
	VIDEO_STREAM_CONFIG_CAPS vcaps;
	AM_MEDIA_TYPE *mt;
	VIDEOINFOHEADER *vih;
	int support = 0;

	if(!pVideoCaptureSource)
		return -1;
	hr = GetUnconnectedPin(pVideoCaptureSource, PINDIR_OUTPUT, &pIPin);
	if(hr)
		return -1;
	hr = pIPin->QueryInterface(IID_IAMStreamConfig, (void **)&psc);
	if(hr)
	{
		pIPin->Release();
		return -1;
	}
	hr = psc->GetNumberOfCapabilities(&count, &size);
	if(hr)
	{
		psc->Release();
		pIPin->Release();
		return -1;
	}
	if(count < max)
		max = count;
	for(i = 0; i < max; i++)
	{
		if(!psc->GetStreamCaps(i, &mt, (BYTE *)&vcaps))
		{
			vih = (VIDEOINFOHEADER *)mt->pbFormat;
			formats[i].xres = vih->bmiHeader.biWidth;
			formats[i].yres = vih->bmiHeader.biHeight;
			if(vih->bmiHeader.biCompression)
				formats[i].fourcc = vih->bmiHeader.biCompression;
			else formats[i].fourcc = vih->bmiHeader.biBitCount;
			DeleteMediaType(mt);
		}
	}
	psc->Release();
	pIPin->Release();
	return max ? max : count;
}

int DXCAPTURE::GetFormats()
{
	IAMStreamConfig *psc;
	HRESULT hr;
	IPin *pIPin;
	int count, i, size;
	VIDEO_STREAM_CONFIG_CAPS vcaps;
	AM_MEDIA_TYPE *mt;
	int support = 0;

	if(!pVideoCaptureSource)
		return -1;
	hr = GetUnconnectedPin(pVideoCaptureSource, PINDIR_OUTPUT, &pIPin);
	if(hr)
		return -1;
	hr = pIPin->QueryInterface(IID_IAMStreamConfig, (void **)&psc);
	if(hr)
	{
		pIPin->Release();
		return -1;
	}
	hr = psc->GetNumberOfCapabilities(&count, &size);
	if(hr)
	{
		psc->Release();
		pIPin->Release();
		return -1;
	}
	for(i = 0; i < count; i++)
	{
		if(!psc->GetStreamCaps(i, &mt, (BYTE *)&vcaps))
		{
			if(mt->subtype == MEDIASUBTYPE_RGB24)
				support |= FMTSUPPORT_RGB24;
			if(mt->subtype == MEDIASUBTYPE_I420)
				support |= FMTSUPPORT_I420;
			if(mt->subtype == MEDIASUBTYPE_YV12)
				support |= FMTSUPPORT_YV12;
			if(mt->subtype == MEDIASUBTYPE_UYVY)
				support |= FMTSUPPORT_UYVY;
			if(mt->subtype == MEDIASUBTYPE_YUY2)
				support |= FMTSUPPORT_YUY2;
			if(mt->subtype == MEDIASUBTYPE_YVU9)
				support |= FMTSUPPORT_YVU9;
			if(mt->subtype == MEDIASUBTYPE_RGB32)
				support |= FMTSUPPORT_RGB32;
			if(mt->subtype == MEDIASUBTYPE_MJPG)
				support |= FMTSUPPORT_MJPG;
			if(mt->subtype == MEDIASUBTYPE_RGB8)
				support |= FMTSUPPORT_RGB8;
			if(mt->subtype == MEDIASUBTYPE_RGB555)
				support |= FMTSUPPORT_RGB555;
			if(mt->subtype == MEDIASUBTYPE_RGB565)
				support |= FMTSUPPORT_RGB565;
			if(mt->subtype == MEDIASUBTYPE_RGB24)
				support |= FMTSUPPORT_RGB24;
			DeleteMediaType(mt);
		}
	}
	psc->Release();
	pIPin->Release();
	return support;
}

int DXCAPTURE::StartCapture()
{
	HRESULT hr;
	int i;
	IPin *pIPin[4];
	AM_MEDIA_TYPE mt;
	VIDEOINFOHEADER vih;
	WAVEFORMATEX wf;
	IAMStreamConfig *psc;

#ifdef USEBDA
	if(usebda)
	{
		bda = new BDAGraph;
		bda->SelectDevice(bdadevice);
		bda->SetResolution(xres, yres);
		bda->SetCaptureCallback(fcb);
		bda->SetFourcc(fourcc);
		if(bdafrequency)
			bda->SetFrequency(bdafrequency);
		if(bdaprogramid)
			bda->SetProgramId(bdaprogramid);
		hr = bda->SubmitDVBTTuneRequest();
		if(!hr)
			running = true;
		else {
			delete bda;
			bda = 0;
		}
		return hr;
	}
#endif
	if(!pVideoCaptureSource && !pAudioCaptureSource)
		return -1;
    if(hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGB))
		return OpenError("Error creating FilterGraph", hr);

#ifdef _WIN32_WCE
    if(hr = CoCreateInstance(CLSID_CaptureGraphBuilder, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pCGB))
		return OpenError("Error creating FilterGraphBuilder2", hr);
#else
    if(hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pCGB))
		return OpenError("Error creating FilterGraphBuilder2", hr);
#endif
	pCGB->SetFiltergraph(pGB);

	// Video
	pIPin[0] = 0;
	if(pVideoCaptureSource)
	{
		hr = pVideoCaptureSource->QueryInterface(IID_IAMDroppedFrames, (void **)&pDroppedFrames);
		if(hr = pGB->AddFilter(pVideoCaptureSource, L"Video Capture Source"))
			return OpenError("Error adding video capture source", hr);
		pVideoCaptureSource->QueryInterface(IID_IAMTimecodeReader, (void **)&pTimecodeReader);
		timecode.streamvalid = false;
#ifdef INCLUDE_DECKLINK
		if(!pVideoCaptureSource->QueryInterface(IID_IDecklinkIOControl, (void **)&pDecklinkIOControl))
		{
			pDecklinkIOControl->SetCaptureTimecodeSource(DECKLINK_TIMECODESOURCE_HANC);
			timecode.counter = 10;
			timecode.source = DECKLINK_TIMECODESOURCE_HANC;
			timecode.streamvalid = true;
			if(hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pTimecodeGrabber))
				return OpenError("Error creating SampleGrabber for timecode", hr);
			if(hr = pTimecodeGrabber->QueryInterface(IID_ISampleGrabber, (void **)&pITimecodeGrabber))
				return OpenError("Error querying ISampleGrabber interface for timecode", hr);
			if(hr = pGB->AddFilter(pTimecodeGrabber, L"Timecode grabber"))
				return OpenError("Error adding SampleGrabber for timecode", hr);
			if(hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pNullTimecodeRenderer))
				return OpenError("Error creating NullRenderer for timecode", hr);
			if(hr = pGB->AddFilter(pNullTimecodeRenderer, L"Null Timecode Renderer"))
				return OpenError("Error adding NullRenderer for timecode", hr);
			pIPin[0] = pIPin[1] = pIPin[2] = pIPin[3] = 0;
			if(hr = pCGB->FindPin(pVideoCaptureSource, PINDIR_OUTPUT, 0, &MEDIATYPE_Timecode, TRUE, 0, &pIPin[0]))
				return OpenError("Timecode pin not found", hr);
			hr = GetUnconnectedPin(pTimecodeGrabber, PINDIR_INPUT, &pIPin[1]);
			if(!hr)
				hr = GetUnconnectedPin(pTimecodeGrabber, PINDIR_OUTPUT, &pIPin[2]);
			if(!hr)
				hr = GetUnconnectedPin(pNullTimecodeRenderer, PINDIR_INPUT, &pIPin[3]);
			if(hr)
			{
				SAFE_RELEASE(pIPin[0]);
				SAFE_RELEASE(pIPin[1]);
				SAFE_RELEASE(pIPin[2]);
				SAFE_RELEASE(pIPin[3]);
				return OpenError("Error getting pins for timecode", hr);
			}
			hr = pGB->Connect(pIPin[0], pIPin[1]);
			hr = pGB->Connect(pIPin[2], pIPin[3]);

			for(i = 0; i < 4; i++)
				SAFE_RELEASE(pIPin[i]);
			if(hr)
				return OpenError("Error connecting pins for timecode", hr);
			pITimecodeGrabber->SetCallback(&TimecodeGrabberCB, 1);
		}
#endif
#ifdef USEDSSINK
		if(hr = CoCreateInstance(CLSID_DSSink, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pNullVideoRenderer))
			return OpenError("Error creating NullRenderer for video", hr);
#else
		if(hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pVideoGrabber))
			return OpenError("Error creating SampleGrabber for video", hr);
		if(hr = pVideoGrabber->QueryInterface(IID_ISampleGrabber, (void **)&pIVideoGrabber))
			return OpenError("Error querying ISampleGrabber interface for video", hr);
		if(hr = pGB->AddFilter(pVideoGrabber, L"Video Grabber"))
			return OpenError("Error adding SampleGrabber for video", hr);
		if(hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pNullVideoRenderer))
			return OpenError("Error creating NullRenderer for video", hr);
#endif
		if(hr = pGB->AddFilter(pNullVideoRenderer, L"Null Video Renderer"))
			return OpenError("Error adding NullRenderer for video", hr);
		ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
		mt.majortype = MEDIATYPE_Video;
		mt.formattype = FORMAT_VideoInfo; 
		mt.bFixedSizeSamples = true;
		mt.cbFormat = sizeof(VIDEOINFOHEADER);
		mt.pbFormat = (BYTE *)&vih;

		ZeroMemory(&vih, sizeof(vih));
		vih.bmiHeader.biCompression = fourcc;
		vih.bmiHeader.biPlanes = 1;
		vih.AvgTimePerFrame = frameduration;
		vih.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih.bmiHeader.biWidth = xres;
		vih.bmiHeader.biHeight = yres;
		switch(fourcc)
		{
		case mmioFOURCC('I','4','2','0'):
			vih.bmiHeader.biBitCount = 12;
			mt.subtype = MEDIASUBTYPE_I420;
			break;
		case mmioFOURCC('Y','V','1','2'):
			vih.bmiHeader.biBitCount = 12;
			vih.bmiHeader.biPlanes = 3;
			vih.bmiHeader.biHeight = yres;
			//vih.bmiHeader.biHeight = -yres;
			mt.subtype = MEDIASUBTYPE_YV12;
#ifdef _WIN32_WCE
			vih.bmiHeader.biCompression = 0x3231d659;	// Y?12 ?
#endif
			mt.lSampleSize = xres*yres*3/2;
			break;
		case mmioFOURCC('U','Y','V','Y'):
			vih.bmiHeader.biBitCount = 16;
			mt.subtype = MEDIASUBTYPE_UYVY;
			mt.lSampleSize = xres*yres*2;
			break;
		case mmioFOURCC('Y','U','Y','2'):
			vih.bmiHeader.biBitCount = 16;
			mt.subtype = MEDIASUBTYPE_YUY2;
			mt.lSampleSize = xres*yres*2;
#ifdef _WIN32_WCE
			vih.bmiHeader.biCompression = 0x3259d559;	// Y?Y2 ?
#endif
			break;
		case mmioFOURCC('Y','V','U','9'):
			vih.bmiHeader.biBitCount = 16;
			mt.subtype = MEDIASUBTYPE_YVU9;
			break;
		case 32:
		case mmioFOURCC('R','G','B','A'):
			vih.bmiHeader.biBitCount = 32;
			mt.subtype = MEDIASUBTYPE_RGB32;
			break;
		case mmioFOURCC('M','J','P','G'):
			vih.bmiHeader.biBitCount = 24;
			mt.subtype = MEDIASUBTYPE_MJPG;
			break;
		case 8:
			vih.bmiHeader.biBitCount = 8;
			mt.subtype = MEDIASUBTYPE_RGB8;
			vih.bmiHeader.biCompression = 0;
			break;
		case 15:
			vih.bmiHeader.biBitCount = 16;
			mt.subtype = MEDIASUBTYPE_RGB555;
#ifdef _WIN32_WCE
			vih.bmiHeader.biCompression = BI_BITFIELDS | BI_SRCPREROTATE;
#else
			vih.bmiHeader.biCompression = BI_BITFIELDS;
#endif
			break;
		case 16:
			vih.bmiHeader.biBitCount = 16;
			mt.subtype = MEDIASUBTYPE_RGB565;
#ifdef _WIN32_WCE
			vih.bmiHeader.biCompression = BI_BITFIELDS | BI_SRCPREROTATE;
#else
			vih.bmiHeader.biCompression = BI_BITFIELDS;
#endif
			break;
		case 0:
		case 24:
		case mmioFOURCC('R','G','B',' '):
			vih.bmiHeader.biBitCount = 24;
			mt.subtype = MEDIASUBTYPE_RGB24;
			vih.bmiHeader.biCompression = 0;
			break;
		}
		vih.bmiHeader.biSizeImage = vih.bmiHeader.biWidth * abs(vih.bmiHeader.biHeight) * vih.bmiHeader.biBitCount / 8;

		pIPin[0] = pIPin[1] = pIPin[2] = pIPin[3] = 0;
		hr = GetUnconnectedPin(pVideoCaptureSource, PINDIR_OUTPUT, &pIPin[0]);
		if(!hr)
		{
			if(hr = pIPin[0]->QueryInterface(IID_IAMStreamConfig, (void **)&psc))
			{
				pIPin[0]->Release();
				return OpenError("Error querying IAMStreamConfig interface for video", hr);
			}
			if(hr = psc->SetFormat(&mt))
			{
 				GUID subtype = mt.subtype;
				mt.subtype = MEDIASUBTYPE_MJPG;
				if(hr = psc->SetFormat(&mt))
				{
					pIPin[0]->Release();
					psc->Release();
					return OpenError("Error setting media type for pVideoCaptureSource output pin", hr);
				}
				psc->Release();
				if(hr = CoCreateInstance(CLSID_MjpegDec, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pMjpegDec))
				{
					pIPin[0]->Release();
					psc->Release();
					return OpenError("Error creating MJPEG decoder", hr);
				}
				if(hr = pGB->AddFilter(pMjpegDec, L"MJPEG decoder"))
				{
					pIPin[0]->Release();
					psc->Release();
					return OpenError("Error adding MJPEG decoder", hr);
				}
				hr = GetUnconnectedPin(pMjpegDec, PINDIR_INPUT, &pIPin[1]);
				if(!hr)
					hr = GetUnconnectedPin(pMjpegDec, PINDIR_OUTPUT, &pIPin[2]);
				if(hr)
				{
					psc->Release();
					SAFE_RELEASE(pIPin[0]);
					SAFE_RELEASE(pIPin[1]);
					SAFE_RELEASE(pIPin[2]);
					return OpenError("Error getting pins for MJPEG decoder", hr);
				}
				hr = pGB->Connect(pIPin[0], pIPin[1]);
				SAFE_RELEASE(pIPin[0]);
				SAFE_RELEASE(pIPin[1]);
				pIPin[0] = pIPin[2];
				pIPin[2] = 0;
				if(hr)
				{
					psc->Release();
					SAFE_RELEASE(pIPin[0]);
					return OpenError("Error connecting pins for MJPEG decoder", hr);
				}
#ifndef USEDSSINK
				mt.subtype = subtype;
				if(hr = pIVideoGrabber->SetMediaType(&mt))
				{
					pIPin[0]->Release();
					return OpenError("Error setting media type for sample grabber", hr);
				}
#endif
			} else psc->Release();
		}
#ifdef USEDSSINK
		if(!hr)
			hr = GetUnconnectedPin(pNullVideoRenderer, PINDIR_INPUT, &pIPin[1]);
		if(hr)
		{
			SAFE_RELEASE(pIPin[0]);
			return OpenError("Error getting pins for video", hr);
		}
		hr = pGB->Connect(pIPin[0], pIPin[1]);
#else
		if(!hr)
			hr = GetUnconnectedPin(pVideoGrabber, PINDIR_INPUT, &pIPin[1]);
		if(!hr)
			hr = GetUnconnectedPin(pVideoGrabber, PINDIR_OUTPUT, &pIPin[2]);
		if(!hr)
			hr = GetUnconnectedPin(pNullVideoRenderer, PINDIR_INPUT, &pIPin[3]);
		if(hr)
		{
			for(i = 0; i < 4; i++)
				SAFE_RELEASE(pIPin[i]);
			return OpenError("Error getting pins for video", hr);
		}
		hr = pGB->Connect(pIPin[0], pIPin[1]);
		hr = pGB->Connect(pIPin[2], pIPin[3]);
#endif
		for(i = 0; i < 4; i++)
			SAFE_RELEASE(pIPin[i]);
		if(hr)
			return OpenError("Error connecting pins for video", hr);
#ifdef USEDSSINK
		IDSSink *dssink;
		pNullVideoRenderer->QueryInterface(IID_IDSSink, (void **)&dssink);
		dssink->SetCallback(this, DSCallback);
#else
		pIVideoGrabber->SetCallback(&VideoGrabberCB, 1);
#endif
		pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, 0, pVideoCaptureSource, IID_IAMTVTuner, (void **)&pTVTuner);
		pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, 0, pVideoCaptureSource, IID_IAMCrossbar, (void **)&pCrossbar);
		pIPin[0] = 0;
		hr = pCGB->FindPin(pVideoCaptureSource, PINDIR_OUTPUT, 0, &MEDIATYPE_Audio, TRUE, 0, &pIPin[0]);
	}

	// Audio
	if(pAudioCaptureSource || pIPin[0])
	{
		if(!pIPin[0])
			if(hr = pGB->AddFilter(pAudioCaptureSource, L"Audio Capture Source"))
				return OpenError("Error adding audio capture source", hr);
#ifdef USEDSSINK
#else
		if(hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pAudioGrabber))
			return OpenError("Error creating SampleGrabber for audio", hr);
		if(hr = pAudioGrabber->QueryInterface(IID_ISampleGrabber, (void **)&pIAudioGrabber))
			return OpenError("Error querying ISampleGrabber interface for audio", hr);
		if(hr = pGB->AddFilter(pAudioGrabber, L"Audio Grabber"))
			return OpenError("Error Adding SampleGrabber for audio", hr);
		if(hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pNullAudioRenderer))
			return OpenError("Error creating NullRenderer for audio", hr);
		if(hr = pGB->AddFilter(pNullAudioRenderer, L"Null Audio Renderer"))
			return OpenError("Error adding NullRenderer for audio", hr);	
#endif
		ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
		mt.majortype = MEDIATYPE_Audio;
		mt.subtype = MEDIASUBTYPE_PCM;
		mt.formattype = FORMAT_WaveFormatEx; 
		mt.cbFormat = sizeof(WAVEFORMATEX);
		mt.bFixedSizeSamples = true;
		mt.lSampleSize = 2 * channels;
		wf.cbSize = sizeof(wf);
		wf.nAvgBytesPerSec = sfreq * channels * 2;
		wf.nBlockAlign = channels * 2;
		wf.nChannels = channels;
		wf.nSamplesPerSec = sfreq;
		wf.wBitsPerSample = 16;
		wf.wFormatTag = 1;
		mt.pbFormat = (BYTE *)&wf;
		
		pIPin[1] = pIPin[2] = pIPin[3] = 0;
		if(!pIPin[0])
			hr = GetUnconnectedPin(pAudioCaptureSource, PINDIR_OUTPUT, &pIPin[0]);
		else hr = 0;
		if(!hr)
		{
			if(hr = pIPin[0]->QueryInterface(IID_IAMStreamConfig, (void **)&psc))
			{
				pIPin[0]->Release();
				return OpenError("Error querying IAMStreamConfig interface for audio", hr);
			}
			if(hr = psc->SetFormat(&mt))
			{
				pIPin[0]->Release();
				psc->Release();
				return OpenError("Error setting media type for pAudioCaptureSource output pin", hr);
			}
			psc->Release();
		}
#ifdef USEDSSINK
		if(!hr)
			hr = GetUnconnectedPin(pNullAudioRenderer, PINDIR_INPUT, &pIPin[1]);
		if(hr)
		{
			SAFE_RELEASE(pIPin[0]);
			hr = pGB->Connect(pIPin[0], pIPin[1]);
		}
#else
		if(!hr)
			hr = GetUnconnectedPin(pAudioGrabber, PINDIR_INPUT, &pIPin[1]);
		if(!hr)
			hr = GetUnconnectedPin(pAudioGrabber, PINDIR_OUTPUT, &pIPin[2]);
		if(!hr)
			hr = GetUnconnectedPin(pNullAudioRenderer, PINDIR_INPUT, &pIPin[3]);
		if(hr)
		{
			for(i = 0; i < 4; i++)
				SAFE_RELEASE(pIPin[i]);
			return OpenError("Error getting pins for audio", hr);
		}
		hr = pGB->Connect(pIPin[0], pIPin[1]);
		hr = pGB->Connect(pIPin[2], pIPin[3]);
#endif
		for(i = 0; i < 4; i++)
			SAFE_RELEASE(pIPin[i]);
		if(hr)
			return OpenError("Error connecting pins for audio", hr);
#ifdef USEDSSINK
#else
		pIAudioGrabber->SetCallback(&AudioGrabberCB, 1);
#endif
	}

	if(hr = pGB->QueryInterface(IID_IMediaControl, (void **)&pMC))
		return OpenError("Error querying IMediaControl interface", hr);

	running = true;
	hr = pMC->Run();
	InterlockedIncrement(&nthreads);

	return hr < 0 ? -1 : 0;
}

#ifdef USEDSSINK
int RegisterDSSink()
{
	TCHAR *p;
	WCHAR path[MAX_PATH];
	HRESULT (WINAPI *DllRegisterServer)(BOOL reg);

	GetModuleFileName(0, path, MAX_PATH);
	p = _tcsrchr(path, '\\');
	if(!p)
		return -1;
	_tcscpy(p, TEXT("\\DSSink.ax"));
	//_tcscpy(p, TEXT("\\..\\DSSink\\DSSink.ax"));
	HINSTANCE hInst = LoadLibrary(path);
	if(!hInst)
		return -1;
	DllRegisterServer = (HRESULT (WINAPI *)(BOOL))GetProcAddress(hInst, TEXT("DllRegisterServer"));
	if(!DllRegisterServer)
	{
		FreeLibrary(hInst);
		return -1;
	}
	if(DllRegisterServer(TRUE))
	{
		FreeLibrary(hInst);
		return -1;
	}
	FreeLibrary(hInst);
	return 0;
}
#endif

DXCAPTURE::DXCAPTURE()
{
	*lasterrorstr = 0;
	pGB = 0;
	pCGB = 0;
	pMC = 0;
	pTVTuner = 0;
	pCrossbar = 0;
	pMjpegDec = 0;
#ifndef USEDSSINK
	pVideoGrabber = 0;
	pAudioGrabber = 0;
	pIVideoGrabber = 0;
	pIAudioGrabber = 0;
#endif
	pNullVideoRenderer = 0;
	pNullAudioRenderer = 0;
	pVideoCaptureSource = 0;
	pAudioCaptureSource = 0;
	pTimecodeReader = 0;
	memset(&timecode, 0, sizeof(timecode));
#ifdef INCLUDE_DECKLINK
	pDecklinkIOControl = 0;
	pTimecodeCaptureSource = 0;
	pNullTimecodeRenderer = 0;
	pITimecodeGrabber = 0;
	pTimecodeGrabber = 0;
	TimecodeGrabberCB.capture = this;
	TimecodeGrabberCB.mode = DXC_MODE_TIMECODE;
#endif
	pDroppedFrames = 0;
	frameduration = 400000;
	channels = 2;
	sfreq = 48000;
	audiotm = videotm = 0;
	fcb = 0;
	running = false;
	status = 0;
	abhead = abtail = 0;
	nthreads = 0;
	AppWindow = 0;
#ifndef USEDSSINK
	VideoGrabberCB.capture = this;
	VideoGrabberCB.mode = DXC_MODE_VIDEO;
	AudioGrabberCB.capture = this;
	AudioGrabberCB.mode = DXC_MODE_AUDIO;
#endif
	xres = 352;
	yres = 288;
	fourcc = 0;
	droppedframes = notdroppedframes = 0;
#ifdef USEBDA
	*bdadevice = 0;
	bda = 0;
	usebda = false;
	bdafrequency = 794000;
	bdaprogramid = 141;
#endif
}

int DXCAPTURE::GetDroppedFramesInfo(int *notdropped, int *dropped)
{
	HRESULT hr;

	if(pDroppedFrames)
	{
		hr = pDroppedFrames->GetNumDropped((long *)dropped);
		hr = pDroppedFrames->GetNumNotDropped((long *)notdropped);
		return 0;
	}
	*notdropped = droppedframes;
	*dropped = droppedframes;
	return -1;
}

int DXCAPTURE::Init(HWND hWnd, UINT uMsg)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	NotifyMsg = uMsg;
	AppWindow = hWnd;
#ifdef USEDSSINK
	return RegisterDSSink();
#else
	return 0;
#endif
}

#ifdef USEDSSINK
void WINAPI DSCallback(void *param, IMediaSample *ms)
{
	BYTE *p;
	long size;
	REFERENCE_TIME st, et;

	ms->GetPointer(&p);
	size = ms->GetSize();
	ms->GetTime(&st, &et);
	((DXCAPTURE *)param)->fcb->VideoFrame((double)st, p, size);
}
#else
long MyISampleGrabberCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	if(capture && capture->fcb)
	{
#ifdef INCLUDE_DECKLINK
		if(mode == DXC_MODE_TIMECODE)
			capture->TimecodeFrame(SampleTime, pBuffer, BufferLen);
		else
#endif
		if(mode == DXC_MODE_AUDIO)
			capture->fcb->AudioFrame(SampleTime, pBuffer, BufferLen);
		else capture->fcb->VideoFrame(SampleTime, pBuffer, BufferLen);
	}
	return S_OK;
}
#endif

#ifdef INCLUDE_DECKLINK
void DXCAPTURE::TimecodeFrame(double SampleTime, BYTE *frame, unsigned bufferlen)
{
	TIMECODE_SAMPLE *tcs;

	tcs = (TIMECODE_SAMPLE *)frame;
	if(tcs->timecode.dwFrames == 0xffffffff)
	{
		timecode.streamvalid = false;
		timecode.counter--;
	} else timecode.streamvalid = true;
	if(timecode.streamvalid)
		timecode.tc = tcs->timecode;
	else if(timecode.counter == 5)
	{
		pDecklinkIOControl->SetCaptureTimecodeSource(DECKLINK_TIMECODESOURCE_VITC);
		timecode.source = DECKLINK_TIMECODESOURCE_VITC;
	} else if(timecode.counter == 0)
	{
		pDecklinkIOControl->SetCaptureTimecodeSource(DECKLINK_TIMECODESOURCE_HANC);
		timecode.source = DECKLINK_TIMECODESOURCE_HANC;
		timecode.counter = 10;
	}
}
#endif

#define BCD2INT(a) (((a>>4)&0xf)*10+(a&0xf))

int DXCAPTURE::GetVcrTimecode(char *text, unsigned *frames, double *framerate)
{
	unsigned f, fr;
	int min;

	if(timecode.streamvalid)
		f = timecode.tc.dwFrames;
	else
	if(pTimecodeReader)
	{
		TIMECODE_SAMPLE tcs;

		memset(&tcs, 0, sizeof(tcs));
		tcs.dwFlags = ED_DEVCAP_TIMECODE_READ;
		if(!pTimecodeReader->GetTimecode(&tcs))
		{
			timecode.tc = tcs.timecode;
			f = tcs.timecode.dwFrames;
		}
	} else return -1;
	switch(timecode.tc.wFrameRate)
	{
	case ED_FORMAT_SMPTE_24:
		fr = 24;
		*framerate = 24.0;
		break;
	case ED_FORMAT_SMPTE_25:
		fr = 25;
		*framerate = 25.0;
		break;
	case ED_FORMAT_SMPTE_30DROP:
		fr = 30;
		*framerate = 29.97;
		break;;
	case ED_FORMAT_SMPTE_30:
		fr = 30;
		*framerate = 30.0;
		break;
	default:
		fr = 0;
		*framerate = 0.0;
	}
	sprintf(text, "%02d:%02d:%02d:%02d", BCD2INT(f>>24), BCD2INT(f>>16), BCD2INT(f>>8), BCD2INT(f));
	*frames = ((BCD2INT(f>>24) * 60 + BCD2INT(f>>16)) * 60 + BCD2INT(f>>8)) * (int)fr + BCD2INT(f);
	if(timecode.tc.wFrameRate == ED_FORMAT_SMPTE_30DROP)
	{
		min = BCD2INT(f>>24) * 60 + BCD2INT(f>>16);
		*frames -= 2 * min - 2 * (min / 10);
	}
	return 0;
}

#ifdef _WIN32_WCE
int DXCAPTURE::EnumCaptureDevices(TCHAR devices[][100], int *ndevices, int mode)
{
#if _MSC_VER <= 1300
	if(*ndevices <= 0)
		return -1;
	_tcscpy(devices[0], TEXT("CAM1:"));
	*ndevices = 1;
#else
	int maxdevices = *ndevices;
	*ndevices = 0;

	if(mode == DXC_MODE_VIDEO)
	{
		DEVMGR_DEVICE_INFORMATION di;
		HANDLE h;

		GUID guidCamera = { 0xCB998A05, 0x122C, 0x4166, 0x84, 0x6A, 0x93, 0x3E, 0x4D, 0x7E, 0x3C, 0x86 };
		di.dwSize = sizeof(di);
		h = FindFirstDevice(DeviceSearchByGuid, &guidCamera, &di);
		if(h == INVALID_HANDLE_VALUE)
			return 0;
		do {
			if(*ndevices < maxdevices)
				_tcscpy(devices[(*ndevices)++], di.szLegacyName);
		} while(FindNextDevice(h, &di));
		FindClose(h);
	}
#endif
	return 0;
}
#else
int DXCAPTURE::EnumCaptureDevices(TCHAR devices[][100], int *ndevices, int mode)
{
	HRESULT hr;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	ULONG cFetched;
	IMoniker *pMoniker = NULL;
	IPropertyBag *pBag;
	int maxdevices = *ndevices;

	*ndevices = 0;
	if(hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum))
		return OpenError("Error creating SystemDeviceEnum", hr);
	if(!(hr = pDevEnum->CreateClassEnumerator(mode == DXC_MODE_AUDIO ? CLSID_AudioInputDeviceCategory : CLSID_VideoInputDeviceCategory, &pClassEnum, 0)))
	{
		while(pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK && cFetched == 1 && *ndevices < maxdevices)
		{
			if(!pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag))
			{
				VARIANT var;

				var.vt = VT_BSTR;
				if(!pBag->Read(L"FriendlyName", &var, NULL))
				{
#ifdef UNICODE					
					_tcscpy(devices[(*ndevices)++], var.bstrVal);
#else
					WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, devices[(*ndevices)++], 100, 0, 0);
#endif
					SysFreeString(var.bstrVal);
				}
				pBag->Release();
			}
			pMoniker->Release();
		}
		pClassEnum->Release();
	}
#ifdef USEBDA
	if(mode == DXC_MODE_VIDEO)
	{
		if(!(hr = pDevEnum->CreateClassEnumerator(KSCATEGORY_BDA_NETWORK_TUNER, &pClassEnum, 0)))
		{
			while(pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK && cFetched == 1 && *ndevices < maxdevices)
			{
				if(!pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag))
				{
					VARIANT var;

					var.vt = VT_BSTR;
					if(!pBag->Read(L"FriendlyName", &var, NULL))
					{
#ifdef UNICODE					
						_tcscpy(devices[(*ndevices)++], var.bstrVal);
#else
						WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, devices[(*ndevices)++], 100, 0, 0);
#endif
						SysFreeString(var.bstrVal);
					}
					pBag->Release();
				}
				pMoniker->Release();
			}
			pClassEnum->Release();
		}
	}
#endif
	pDevEnum->Release();
	return 0;
}
#endif

int DXCAPTURE::StopCapture()
{
	if(!running)
		return 0;
	if(pMC)
		pMC->Stop();
	notdroppedframes = droppedframes = 0;
	GetDroppedFramesInfo(&notdroppedframes, &droppedframes);
	SAFE_RELEASE(pMC);
	SAFE_RELEASE(pNullAudioRenderer);
#ifndef USEDSSINK
	SAFE_RELEASE(pIAudioGrabber);
	SAFE_RELEASE(pAudioGrabber);
#endif
	SAFE_RELEASE(pNullVideoRenderer);
#ifdef INCLUDE_DECKLINK
	SAFE_RELEASE(pDecklinkIOControl);
	SAFE_RELEASE(pTimecodeCaptureSource);
	SAFE_RELEASE(pNullTimecodeRenderer);
	SAFE_RELEASE(pITimecodeGrabber);
	SAFE_RELEASE(pTimecodeGrabber);
#endif
	SAFE_RELEASE(pTimecodeReader);
#ifndef USEDSSINK
	SAFE_RELEASE(pIVideoGrabber);
	SAFE_RELEASE(pVideoGrabber);
#endif
	SAFE_RELEASE(pDroppedFrames);
	SAFE_RELEASE(pMjpegDec);
	SAFE_RELEASE(pTVTuner);
	SAFE_RELEASE(pCrossbar);
	SAFE_RELEASE(pGB);
	SAFE_RELEASE(pCGB);
#ifdef USEBDA
	if(bda)
	{
		delete bda;
		bda = 0;
	}
#endif
	return 0;
}

DXCAPTURE::~DXCAPTURE()
{
	StopCapture();
	SAFE_RELEASE(pVideoCaptureSource);
	SAFE_RELEASE(pAudioCaptureSource);
}

#ifdef _WIN32_WCE
int DXCAPTURE::SelectDevice(const TCHAR *device, int mode)
{
	if(mode == DXC_MODE_VIDEO)
	{
		IPersistPropertyBag *pb;
		HRESULT hr;

		if(hr = CoCreateInstance(CLSID_VideoCapture, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pVideoCaptureSource))
			return OpenError("Error creating video capture source", hr);
		if(!pVideoCaptureSource->QueryInterface(IID_IPersistPropertyBag, (void **)&pb))
		{
			HRESULT hr;
			CComVariant camname = device;
			CPropertyBag PropBag;
			PropBag.Write(TEXT("VCapName"), &camname);
			if((hr = pb->Load(&PropBag, NULL)))
			{
				pb->Release();
				return -1;
			}
			pb->Release();
			return 0;
		}
	}
	return -1;
}
#else
int DXCAPTURE::SelectDevice(const TCHAR *device, int mode)
{
	HRESULT hr;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	ULONG cFetched;
	IMoniker *pMoniker = NULL;
	IPropertyBag *pBag;
	int rc = -1;

	if(mode == DXC_MODE_AUDIO)
	{
		SAFE_RELEASE(pAudioCaptureSource);
	} else {
		SAFE_RELEASE(pVideoCaptureSource);
	}
	if(hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum))
		return OpenError("Error creating SystemDeviceEnum", hr);
	if(!(hr = pDevEnum->CreateClassEnumerator(mode == DXC_MODE_AUDIO ? CLSID_AudioInputDeviceCategory : CLSID_VideoInputDeviceCategory, &pClassEnum, 0)))
	{
		while(pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK && cFetched == 1 && rc == -1)
		{
			if(!pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag))
			{
				VARIANT var;

				var.vt = VT_BSTR;
				if(!pBag->Read(L"FriendlyName", &var, NULL))
				{
#ifdef UNICODE					
					if(!_tcscmp(device, var.bstrVal))
#else
					char dev[100];
					WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, dev, 100, 0, 0);
					if(!strcmp(device, dev))
#endif
					{
						pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)(mode == DXC_MODE_AUDIO ? &pAudioCaptureSource : &pVideoCaptureSource));
						rc = 0;
					}
					SysFreeString(var.bstrVal);
				}
				pBag->Release();
			}
			pMoniker->Release();
		}
		pClassEnum->Release();
	}
#ifdef USEBDA
	if(mode == DXC_MODE_VIDEO)
	{
		if(rc == -1)
		{
			strcpy(bdadevice, device);
			rc = 0;
			usebda = 1;
		} else usebda = 0;
	}
#endif
	pDevEnum->Release();
	if(rc == -1)
		return OpenError("Error creating enumerator for capture devices", hr);
	return rc;
}
#endif

int DXCAPTURE::SettingsWindow(HWND hWnd, int what)
{
#ifdef _WIN32_WCE
	return -1;
#else
#ifndef _WIN32_WCE
	IAMVfwCaptureDialogs *pVfwDlg;
#endif
	HRESULT hr;
	bool wasrunning = running;
	bool formatchanged = false;
	ISpecifyPropertyPages *pSpec;
	CAUUID cauuid;
	IPin *pin;
	ICaptureGraphBuilder *pCGB;
	IBaseFilter *pCrossbar, *pTVTuner;

	if(!pVideoCaptureSource)
		return -1;
	if(running)
		StopCapture();
    if(hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGB))
	{
		if(wasrunning)
			StartCapture();
		return -1;
	}
    if(hr = CoCreateInstance(CLSID_CaptureGraphBuilder, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder, (void **)&pCGB))
	{
		SAFE_RELEASE(pGB);
		if(wasrunning)
			StartCapture();
		return -1;
	}
	pCGB->SetFiltergraph(pGB);
	if(hr = pGB->AddFilter(pVideoCaptureSource, L"Video Capture Source"))
	{
		SAFE_RELEASE(pGB);
		SAFE_RELEASE(pCGB);
		if(wasrunning)
			StartCapture();
		return -1;
	}
#ifndef _WIN32_WCE
	if(!pVideoCaptureSource->QueryInterface(IID_IAMVfwCaptureDialogs, (void **)&pVfwDlg))
	{
		if(!pVfwDlg->ShowDialog(what, hWnd))
			formatchanged = true;
		pVfwDlg->Release();
		what = 0;
	}
#endif

	if(what == 1 && !pVideoCaptureSource->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))
	{
		if(!pSpec->GetPages(&cauuid))
		{
			if(!OleCreatePropertyFrame(hWnd, 30, 30, NULL, 1, (IUnknown **)&pVideoCaptureSource, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL))
				formatchanged = true;
			CoTaskMemFree(cauuid.pElems);
			pSpec->Release();
		}
	}
	if(what == 2 && !GetUnconnectedPin(pVideoCaptureSource, PINDIR_OUTPUT, &pin))
	{
		if(!pin->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))
		{
			if(!pSpec->GetPages(&cauuid))
			{
				if(!OleCreatePropertyFrame(hWnd, 30, 30, NULL, 1, (IUnknown **)&pin, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL))
					formatchanged = true;
				CoTaskMemFree(cauuid.pElems);
				pSpec->Release();
			}
		}
		pin->Release();
	}

	if(what == 3 && !pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, pVideoCaptureSource, IID_IAMCrossbar, (void **)&pCrossbar))
	{
		if(!pCrossbar->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))
		{
			if(!pSpec->GetPages(&cauuid))
			{
				if(!OleCreatePropertyFrame(hWnd, 30, 30, NULL, 1, (IUnknown **)&pCrossbar, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL))
					formatchanged = true;
				CoTaskMemFree(cauuid.pElems);
				pSpec->Release();
			}
		}
		pCrossbar->Release();
	}

	if(what == 4 && !pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, pVideoCaptureSource, IID_IAMTVTuner, (void **)&pTVTuner))
	{
		if(!pTVTuner->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec))
		{
			if(!pSpec->GetPages(&cauuid))
			{
				if(!OleCreatePropertyFrame(hWnd, 30, 30, NULL, 1, (IUnknown **)&pTVTuner, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL))
					formatchanged = true;
				CoTaskMemFree(cauuid.pElems);
				pSpec->Release();
			}
		}
		pTVTuner->Release();
	}

	if(formatchanged)
	{
		IPin *pin;
		IAMStreamConfig *sc;
		AM_MEDIA_TYPE *mt;
		VIDEOINFOHEADER *pvih;
	
		if(!GetUnconnectedPin(pVideoCaptureSource, PINDIR_OUTPUT, &pin))
		{
			if(!pin->QueryInterface(IID_IAMStreamConfig, (void **)&sc))
			{
				if(!sc->GetFormat(&mt))
				{
					pvih = (VIDEOINFOHEADER *)mt->pbFormat;
					xres = pvih->bmiHeader.biWidth;
					yres = pvih->bmiHeader.biHeight;
					fourcc = pvih->bmiHeader.biCompression;
					if(!fourcc)
					{
						if(pvih->bmiHeader.biBitCount == 32)
							fourcc = mmioFOURCC('R','G','B','A');
						else if(pvih->bmiHeader.biBitCount == 16 && mt->subtype == MEDIASUBTYPE_RGB555)
							fourcc = 15;
						else fourcc = pvih->bmiHeader.biBitCount;
					}
					if(mt->subtype == MEDIASUBTYPE_YV12)
						fourcc = mmioFOURCC('Y','V','1','2');
					DeleteMediaType(mt);
				}
				sc->Release();
			}
			pin->Release();
		}
	}

	SAFE_RELEASE(pCGB);
	SAFE_RELEASE(pGB);
	if(wasrunning)
		StartCapture();
	return 0;
#endif
}

int DXCAPTURE::SetVideoFormat(int xres, int yres, double fps, DWORD fourcc)
{
	this->xres = xres;
	this->yres = yres;
	this->fourcc = fourcc;
	frameduration = (unsigned)(10000000.0 / fps);
	return 0;
}

int DXCAPTURE::GetVideoFormat(int *xres, int *yres, double *fps, DWORD *fourcc)
{
	if(xres)
		*xres = this->xres;
	if(yres)
		*yres = this->yres;
	if(fourcc)
		*fourcc = this->fourcc;
	if(fps)
		*fps = 10000000.0 / frameduration;
	return 0;
}

int DXCAPTURE::SetAudioFormat(int channels, int sfreq)
{
	this->channels = channels;
	this->sfreq = sfreq;
	return 0;
}

int DXCAPTURE::GetAudioFormat(int *channels, int *sfreq)
{
	if(channels)
		*channels = this->channels;
	if(sfreq)
		*sfreq = this->sfreq;
	return 0;
}

int DXCAPTURE::GetCaptureFilter(IBaseFilter **filter)
{
	*filter = pVideoCaptureSource;
	if(*filter)
		return 0;
	return -1;
}

static const int frequencies[100] ={
294250, 46250, 48250, 55250, 62250, 175250, 182250, 189250, 196250, 203250,
210250, 217250, 224250, 53250, 62250, 82250, 175250, 183250, 192250, 201250,
210250, 471250, 479250, 487250, 495250, 503250, 511250, 519250, 527250, 535250,
543250, 551250, 559250, 567250, 575250, 583250, 591250, 599250, 607250, 615250,
623250, 631250, 639250, 647250, 655250, 663250, 671250, 679250, 687250, 695250,
703250, 711250, 719250, 727250, 735250, 743250, 751250, 759250, 767250, 775250,
783250, 791250, 799250, 807250, 815250, 823250, 831250, 839250, 847250, 855250,
863250, 871250, 879250, 879250, 69250, 76250, 83250, 90250, 97250, 59250,
93250, 105250, 112250, 119250, 126250, 133250, 140250, 147250, 154250, 161250,
168250, 231250, 238250, 247250, 252250, 259250, 266250, 273250, 280250, 287250};

int DXCAPTURE::IsBDA()
{
#ifdef USEBDA
	return usebda;
#else
	return 0;
#endif
}

int DXCAPTURE::SetChannel(int ch)
{
#ifdef USEBDA
	if(usebda)
	{
		bdaprogramid = 0;
		if(ch > 1000)
			bdafrequency = ch;
		else if(ch >= 0 && ch <= 99)
			bdafrequency = frequencies[ch]+2750;
		else return -1;
		if(bda)
			bda->SetFrequency(bdafrequency);
		return 0;
	}
#endif
	if(!pTVTuner)
		return -1;
	if(ch >= 101)
	{
		ch -= 100;
		pTVTuner->put_CountryCode(41);	// Switzerland
		pTVTuner->put_InputType(0, TunerInputCable);
		pTVTuner->put_Channel(ch - 100, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
		return 0;
	}
	if(ch == 2)
		ch = 1;
	pTVTuner->put_InputType(0, TunerInputAntenna);
	if(ch >= 13 && ch <= 20)
	{
		pTVTuner->put_CountryCode(39);	// Italy
		ch -= 12;
	} else pTVTuner->put_CountryCode(41);	// Switzerland
	pTVTuner->put_Channel(ch, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
	return 0;
}

#ifdef USEBDA
int DXCAPTURE::ProgramsList(char *list, int size)
{
	if(bda)
		return bda->ProgramsList(list, size);
	return -1;
}

int DXCAPTURE::SetProgramId(int id)
{
	bdaprogramid = id;
	if(bda)
		bda->SetProgramId(id);
	return 0;
}

#endif
