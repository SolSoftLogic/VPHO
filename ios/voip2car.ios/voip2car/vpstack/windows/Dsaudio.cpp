#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include "../vpaudio.h"
#include "../portability.h"

static bool ds_initialized;
static HRESULT (WINAPI *DirectSoundEnumerateM)(LPDSENUMCALLBACK lpCallback, LPVOID lpContext);
static HRESULT (WINAPI *DirectSoundCreateM)(LPGUID lpGuid, LPDIRECTSOUND *ppDS, IUnknown *pUnkOuter);
static GUID ds_guids[100];
static TCHAR ds_devnames[100][100];
static unsigned ds_ndevices;
static unsigned ds_usbdevice;
static CRITICAL_SECTION cs;

HWND ds_AppWindow;

static void wipespaces(TCHAR *s)
{
	TCHAR *p;

	p = s + _tcslen(s) - 1;
	while(p > s && *p == ' ')
		*p-- = 0;
	for(p = s; *p == ' '; p++);
	if(p > s)
		memmove(s, p, (_tcslen(p) + 1) * sizeof(TCHAR));
}

BOOL CALLBACK DsEnumCallBack(GUID *lpGuid, TCHAR *desc, TCHAR *module, void *dummy)
{
	if(!lpGuid)
		return TRUE;
	memcpy(&ds_guids[ds_ndevices], lpGuid, sizeof(GUID));
	_tcsncpy(ds_devnames[ds_ndevices], desc, 99);
	wipespaces(ds_devnames[ds_ndevices]);
	if(!_tcsicmp(module, TEXT("usbaudio.sys")))
		ds_usbdevice = ds_ndevices;
	ds_ndevices++;
	return TRUE;
}

void InitializeDS()
{
	HINSTANCE hDSoundDll;

	if(ds_initialized)
		return;
	if(!ds_AppWindow)
		ds_AppWindow = GetDesktopWindow();
	hDSoundDll = LoadLibrary(TEXT("Dsound.dll"));
	if(hDSoundDll)
	{
#ifdef UNICODE
		DirectSoundEnumerateM = (HRESULT (WINAPI *)(LPDSENUMCALLBACK, LPVOID))GetProcAddress(hDSoundDll, TEXT("DirectSoundEnumerateW"));
#else
		DirectSoundEnumerateM = (HRESULT (WINAPI *)(LPDSENUMCALLBACK, LPVOID))GetProcAddress(hDSoundDll, "DirectSoundEnumerateA");
#endif
		DirectSoundCreateM = (HRESULT (WINAPI *)(LPGUID, LPDIRECTSOUND *, IUnknown *))GetProcAddress(hDSoundDll, TEXT("DirectSoundCreate"));
//		MessageBox(0, TEXT("DSound.dll ok"), TEXT(""), 0);
	} else MessageBox(0, TEXT("DSound.dll not found"), TEXT(""), 0);
	CoInitialize(0);
	InitializeCriticalSection(&cs);
	ds_initialized = true;
}

static void DS_EnumerateDevices()
{
	ds_ndevices = 0;
	ds_usbdevice = 0;
	if(DirectSoundEnumerateM)
		DirectSoundEnumerateM((LPDSENUMCALLBACK)DsEnumCallBack, 0);
}

int ds_USBDeviceName(TCHAR *devicename)
{
	*devicename = 0;
	if(ds_ndevices)
	{
		_tcscpy(devicename, ds_devnames[ds_usbdevice]);
		return 0;
	}
	return -1;
}

TCHAR *ds_GetDeviceName(int i)
{
	if(!i)
	{
		EnterCriticalSection(&cs);
		DS_EnumerateDevices();
		LeaveCriticalSection(&cs);
	}
	if(i >= (int)ds_ndevices)
		return 0;
	return ds_devnames[i];
}

tAudioOutDS::tAudioOutDS()
{
	EnterCriticalSection(&cs);
	DS_EnumerateDevices();
	LeaveCriticalSection(&cs);
	maxdelay = 500;
	sfreq = 0;
	dsound = 0;
	dsbuf = 0;
	idev = 0;
	imixer = -1;
	outgain = 0;
	playing = false;
	stopping = false;
	terminating = false;
}

tAudioOutDS::~tAudioOutDS()
{
	terminating = true;
	Stop();
}

int tAudioOutDS::SetAudioDevice(TCHAR *sdev)
{
	int i, n, newidev;
	MIXERCAPS mcaps;

	imixer = -1;
	EnterCriticalSection(&cs);
	DS_EnumerateDevices();
	for(i = 0; i < (int)ds_ndevices; i++)
		if(!_tcscmp(sdev, ds_devnames[i]))
			break;
	if(i == (int)ds_ndevices)
	{
		idev = 0;
		LeaveCriticalSection(&cs);
		return -1;
	}
	LeaveCriticalSection(&cs);
	newidev = i;
	n = mixerGetNumDevs();
	for(i = 0; i < n; i++)
	{
		mixerGetDevCaps(i, &mcaps, sizeof(mcaps));
		if(!_tcscmp(mcaps.szPname, sdev))
			break;
	}
	if(i != n)
		imixer = i;
	if(newidev == (int)idev)
		return 0;
	idev = newidev;
	Stop();
	return 0;
}

int tAudioOutDS::CheckAudioDevice(TCHAR *sdev)
{
	int i;

	EnterCriticalSection(&cs);
	DS_EnumerateDevices();
	for(i = 0; i < (int)ds_ndevices; i++)
		if(!_tcscmp(sdev, ds_devnames[i]))
			break;
	if(i == (int)ds_ndevices)
	{
		if(ds_ndevices)
			_tcscpy(sdev, ds_devnames[0]);
		else *sdev = 0;
		LeaveCriticalSection(&cs);
		return -1;
	}
	LeaveCriticalSection(&cs);
	return 0;
}

int tAudioOutDS::Stop()
{
	EnterCriticalSection(&cs);
	if(stopping)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}
	stopping = true;
	if(dsbuf)
	{
		delete[] vumeter;
		dsbuf->Release();
		dsbuf = 0;
	}
	if(dsound)
	{
		dsound->Release();
		dsound = 0;
	}
	sfreq = 0;
	LeaveCriticalSection(&cs);
	while(playing)
		Sleep(10);
	stopping = false;
	return 0;
}

int tAudioOutDS::Start(int channels, unsigned sfreq)
{
	DSBUFFERDESC bd;
	WAVEFORMATEX fmt;

	if(stopping)
		return -1;
	if(!DirectSoundCreateM || DirectSoundCreateM(&ds_guids[idev], &dsound, 0) != DS_OK)
		return -1;
	if(dsound->SetCooperativeLevel(ds_AppWindow, DSSCL_PRIORITY) != DS_OK)
	{
		dsound->Release();
		dsound = 0;
		return -1;
	}
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nChannels = channels;
	fmt.nSamplesPerSec = sfreq;
	fmt.nAvgBytesPerSec = 2 * channels * sfreq;
	fmt.nBlockAlign = 2 * channels;
	fmt.wBitsPerSample = 16;
	fmt.cbSize = sizeof(WAVEFORMATEX);
	memset(&bd, 0, sizeof(bd));
	bd.dwSize = sizeof(bd);
	bd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	if(maxdelay < 50)
		maxdelay = 50;
	bd.dwBufferBytes = buffersize = 5 * sfreq * channels;	// 2.5 seconds max buffer length
	bd.lpwfxFormat = &fmt;
	if(dsound->CreateSoundBuffer(&bd, &dsbuf, 0) != DS_OK)
	{
		dsound->Release();
		dsound = 0;
		return -1;
	}
	vumeter = new unsigned short [buffersize * 50 / sfreq];
	ZeroMemory(vumeter, 2 * buffersize * 50 / sfreq);
	writepos = zeroedpos = 0;
	prevwritecursor = 0;
	this->sfreq = sfreq;
	return 0;
}

int tAudioOutDS::PutData(int channels, unsigned sfreq, short *buf, unsigned len)
{
	int x;
	unsigned i, halflen, sampleptr, buffersizehalf;
	void *ptr[2];
	DWORD blen[2], blenhalf[2];
	DWORD writecursor;

	if(!sfreq || !len || !buf || terminating || stopping)
		return -1;
	halflen = len / 2;
	EnterCriticalSection(&cs);
	if(sfreq != this->sfreq)
	{
		LeaveCriticalSection(&cs);
		Stop();
		if(terminating)
			return -1;
		EnterCriticalSection(&cs);
		if(Start(channels, sfreq))
		{
			LeaveCriticalSection(&cs);
			return -1;
		}
	}
	writecursor = 0;
	if(playing)
		dsbuf->GetCurrentPosition(0, &writecursor);
	if((writecursor - prevwritecursor + buffersize) % buffersize > (writepos - prevwritecursor + buffersize) % buffersize)
		writepos = writecursor;		// Hole (underrun)
	prevwritecursor = writecursor;
	if((writepos - writecursor + buffersize) % buffersize / 2 > maxdelay * sfreq / 1000)
	{
		// Delay exceeded: drop frame
		LeaveCriticalSection(&cs);
		return -2;
	}
	if(dsbuf->Lock(writepos, len, &ptr[0], &blen[0], &ptr[1], &blen[1], 0) == DS_OK)
	{
		if(blen[0] + blen[1] == len)
		{
			if(outgain)
			{
				blenhalf[0] = blen[0] / 2;
				blenhalf[1] = blen[1] / 2;
				for(i = 0; i < blenhalf[0]; i++)
				{
					x = buf[i] * 3;
					if(x > 32767)
						x = 32767;
					else if(x < -32768)
						x = -32768;
					((short *)(ptr[0]))[i] = (short)x;
				}
				for(i = 0; i < blenhalf[1]; i++)
				{
					x = buf[i + blenhalf[0]] * 3;
					if(x > 32767)
						x = 32767;
					else if(x < -32768)
						x = -32768;
					((short *)(ptr[1]))[i] = (short)x;
				}					
			} else {
				memcpy(ptr[0], buf, blen[0]);
				if(ptr[1])
					memcpy(ptr[1], (char *)buf + blen[0], blen[1]);
			}
			dsbuf->Unlock(ptr[0], blen[0], ptr[1], blen[1]);
			sampleptr = writepos / 2;
			buffersizehalf = buffersize / 2;
			for(i = 0; i < halflen; i++)
			{
				x = (sampleptr + i) % buffersizehalf * 100 / sfreq;
				if(buf[i] > vumeter[x])
					vumeter[x] = buf[i];
				else if(-buf[i] > vumeter[x])
					vumeter[x] = -buf[i];
			}
			writepos = (writepos + len) % buffersize;
			if(!playing)
			{
				playing = 1;
				StartPlaying();
			}
		} else {
			dsbuf->Unlock(ptr[0], blen[0], ptr[1], blen[1]);
			LeaveCriticalSection(&cs);
			return -1;
		}
	}
	LeaveCriticalSection(&cs);
	OutputDebugString("\r\n");
	return 0;
}

static void dsthread(void *object)
{
	((tAudioOutDS *)object)->thread();
}

void tAudioOutDS::thread()
{
	DWORD playcursor, writecursor;
	void *ptr[2];
	DWORD blen[2];
	unsigned len;

	EnterCriticalSection(&cs);
	if(!dsbuf || stopping)
	{
		LeaveCriticalSection(&cs);
		playing = 0;
		return;
	}
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	dsbuf->Play(0, 0, DSBPLAY_LOOPING);
	playing = 2;
	while(!stopping)
	{
		playcursor = writecursor = 0;	// In case that the next call fails (USB device disappears)
		dsbuf->GetCurrentPosition(&playcursor, &writecursor);
		if((writecursor - prevwritecursor + buffersize) % buffersize > (writepos - prevwritecursor + buffersize) % buffersize)
			writepos = writecursor;		// Hole (underrun)
		prevwritecursor = writecursor;
		if(playcursor != zeroedpos)
		{
			if(zeroedpos > playcursor)
			{
				ZeroMemory(vumeter + zeroedpos * 50 / sfreq, (buffersize - zeroedpos) * 100 / sfreq);
				ZeroMemory(vumeter, playcursor * 100 / sfreq);
			} else ZeroMemory(vumeter + zeroedpos * 50 / sfreq, (playcursor - zeroedpos) * 100 / sfreq);
			len = (playcursor - zeroedpos + buffersize) % buffersize;
			if(dsbuf->Lock(zeroedpos, len, &ptr[0], &blen[0], &ptr[1], &blen[1], 0) == DS_OK)
			{
				ZeroMemory(ptr[0], blen[0]);
				if(blen[1])
					ZeroMemory(ptr[1], blen[1]);
				zeroedpos = playcursor;
				dsbuf->Unlock(ptr[0], blen[0], ptr[1], blen[1]);
			}
		}
		LeaveCriticalSection(&cs);
		Sleep(100);
		EnterCriticalSection(&cs);
	}
	playing = 0;
	LeaveCriticalSection(&cs);
}

void tAudioOutDS::StartPlaying()
{
	_beginthread(dsthread, 0, this);
}

int tAudioOutDS::SetBufferLength(unsigned ms)
{
	if(ms < 50)
		maxdelay = 30;
	else if(ms > 2000)
		maxdelay = 2000;
	else maxdelay = ms;
	return 0;
}

int tAudioOutDS::GetCurrentDelay()
{
	DWORD playcursor, writecursor;
	int delay;

	EnterCriticalSection(&cs);
	if(!sfreq || !playing)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	playcursor = writecursor = 0;
	dsbuf->GetCurrentPosition(&playcursor, &writecursor);
	if((writecursor - prevwritecursor + buffersize) % buffersize > (writepos - prevwritecursor + buffersize) % buffersize)
		writepos = writecursor;		// Hole (underrun)
	prevwritecursor = writecursor;
	delay = ((writepos - writecursor + buffersize) % buffersize) * 1000 / sfreq / 2;
	LeaveCriticalSection(&cs);
	return delay;
}

int tAudioOutDS::VuMeter()
{
	DWORD playcursor;
	int vu;

	EnterCriticalSection(&cs);
	if(!sfreq || playing != 2)
	{
		LeaveCriticalSection(&cs);
		return -10000;
	}
	playcursor = 0;
	dsbuf->GetCurrentPosition(&playcursor, 0);
	vu = vumeter[playcursor * 50 / sfreq];
	LeaveCriticalSection(&cs);
	if(vu)
		return (int)(2000.0*log10(vu / 32678.0));
	else return -10000;
}

int tAudioOutDS::Beep(int sfreqmultiplier)
{
	short frame[800];
	int i;

	for(i = 0; i < 400 * sfreqmultiplier; i++)
		frame[i] = (short)(10000 * sin(i / (2.0 * sfreqmultiplier)));
	PutData(1, 8000 * sfreqmultiplier, frame, 800 * sfreqmultiplier);
	return 0;
}

int mixerSetPlayLevel(HMIXER hmx, int componenttype, int thousands)
{
	MIXERLINE mxl;
	MIXERLINECONTROLS mxlc;
	MIXERCONTROL mxc[10];
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED volume[2];
	MIXERCAPS mxcaps;
	TCHAR selectname[100];
	int rc, connections, canselect = 0;
	unsigned i;

	if(mixerGetDevCaps((UINT)hmx, &mxcaps, sizeof(mxcaps)) != MMSYSERR_NOERROR)
		return -1;
	mxl.cbStruct = sizeof(mxl);
	for(mxl.dwDestination = 0; mxl.dwDestination < mxcaps.cDestinations; mxl.dwDestination++)
	{
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
			return -1;
		if(mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
			break;
	}
	connections = mxl.cConnections;
	mxlc.cbStruct = sizeof(mxlc);
	mxlc.cControls = mxl.cControls > 10 ? 10 : mxl.cControls;
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.cbmxctrl = sizeof(mxc[0]);
	mxlc.pamxctrl = mxc;
	if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ALL | MIXER_OBJECTF_HMIXER)) == MMSYSERR_NOERROR)
	for(mxl.dwSource = 0; mxl.dwSource < (unsigned)connections; mxl.dwSource++)
	{
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
			continue;
		if(mxl.dwComponentType != (unsigned)componenttype)
			continue;
		_tcscpy(selectname, mxl.szName);
		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		mxlc.cControls = mxl.cControls > 10 ? 10 : mxl.cControls;
		mxlc.cbmxctrl = sizeof(mxc[0]);
		mxlc.pamxctrl = mxc;
		if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
			continue;
		mxcd.cbStruct = sizeof(mxcd);
		mxcd.dwControlID = mxc[0].dwControlID;
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = 0;
		mxcd.cbDetails = sizeof(volume);
		mxcd.paDetails = volume;
		volume[0].dwValue = (mxc[0].Bounds.dwMaximum - mxc[0].Bounds.dwMinimum) * thousands / 1000 + mxc[0].Bounds.dwMinimum;
		volume[1].dwValue = (mxc[0].Bounds.dwMaximum - mxc[0].Bounds.dwMinimum) * thousands / 1000 + mxc[0].Bounds.dwMinimum;
		if((rc = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
			continue;
		return 0;
	}
	for(i = 0; i < mxlc.cControls; i++)
	{
		if(mxc[i].dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)
		{
			mxcd.cbStruct = sizeof(mxcd);
			mxcd.dwControlID = mxc[i].dwControlID;
			mxcd.cChannels = 1;
			mxcd.cMultipleItems = 0;
			mxcd.cbDetails = sizeof(volume);
			mxcd.paDetails = volume;
			volume[0].dwValue = (mxc[i].Bounds.dwMaximum - mxc[i].Bounds.dwMinimum) * thousands / 1000 + mxc[i].Bounds.dwMinimum;
			volume[1].dwValue = (mxc[i].Bounds.dwMaximum - mxc[i].Bounds.dwMinimum) * thousands / 1000 + mxc[i].Bounds.dwMinimum;
			if((rc = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
				continue;
			return 0;
		}
	}
	return -1;
}

int tAudioOutDS::SetVolume(int thousands)
{
	HMIXER	hmixer;

	if(imixer == -1)
		return -1;
	if(mixerOpen(&hmixer, imixer, 0, 0, MIXER_OBJECTF_MIXER) != MMSYSERR_NOERROR)
		return -1;
	mixerSetPlayLevel(hmixer, MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, thousands);
	mixerClose(hmixer);
	return 0;
}
