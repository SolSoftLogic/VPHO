#include <objbase.h>
#include "DSSink.h"

#define INPUT_PIN_NAME	TEXT("DSSink Input")
#define FILTER_NAME		TEXT("DSSink Filter")
#define DLL_NAME		TEXT("DSSink")

class InputPin;
class Filter : public CBaseFilter, public IDSSink
{
public:
	DECLARE_IUNKNOWN;

	static CUnknown *CreateInstance(IUnknown *pUnknown, HRESULT *phr);
	Filter(TCHAR *name, IUnknown *pUnknown, HRESULT *phr);
	~Filter();
	CBasePin *GetPin(int index);
	STDMETHOD(SetCallback)(void *param, void (WINAPI *SampleCallback)(void *param, IMediaSample *sample));
	int GetPinCount();
	AMOVIESETUP_FILTER *GetSetupData();
	STDMETHODIMP NonDelegatingQueryInterface(const IID &riid, void **ppv);
protected:
	CCritSec cs;
	InputPin *pin;
};

class InputPin : public CBaseInputPin
{
public:
	void (WINAPI *SampleCallback)(void *param, IMediaSample *sample);
	void *param;
	InputPin(TCHAR *pClassName, Filter *pFilter, CCritSec *pcs, HRESULT *phr, TCHAR *pPinName);
	~InputPin();
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	STDMETHODIMP Receive(IMediaSample *pSample);
};

const AMOVIESETUP_MEDIATYPE f_InputType[] =
{
    {
        &MEDIATYPE_NULL,
        &MEDIASUBTYPE_NULL
    }
};

const AMOVIESETUP_PIN f_Pins[] =
{
    {
        INPUT_PIN_NAME,
        FALSE,                          // bRendered
        FALSE,                          // bOutput
        FALSE,                          // bZero
        FALSE,                          // bMany
        &CLSID_NULL,                    // clsConnectsToFilter
        NULL,                           // ConnectsToPin
        NUMELMS(f_InputType),           // Number of media types
        f_InputType
    }
};

AMOVIESETUP_FILTER f_Filter =
{
    &CLSID_DSSink,
    FILTER_NAME,
    MERIT_DO_NOT_USE,
    NUMELMS(f_Pins),
    f_Pins
} ;

// Global template array. AMovieDllRegisterServer2 in the base
// class uses this to determine which filters to register.
CFactoryTemplate g_Templates[] =
{
    {
        FILTER_NAME,
        &CLSID_DSSink,
        Filter::CreateInstance,
        NULL,
        &f_Filter
    }
};

// Global count of # of templates. The base class uses this to
// figure out the # of entries in g_Templates
int g_cTemplates = NUMELMS(g_Templates);

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
		_tcscpy(dpCurSettings.lpszName, DLL_NAME);

	BOOL Succeeded = DllEntryPoint(static_cast<HINSTANCE>(hModule),	dwReason, lpReserved);
	if(!Succeeded)
		return FALSE;
	return TRUE;
}

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

CUnknown* Filter::CreateInstance(IUnknown *pUnknown, HRESULT *phr)
{
	*phr = S_OK;

	Filter* pFilter = new Filter(FILTER_NAME, pUnknown,	phr);
	if(!pFilter)
		*phr = E_OUTOFMEMORY;
	if(FAILED(*phr))
	{
		delete pFilter;
		pFilter = NULL;
	}
	return pFilter;
}

Filter::Filter(TCHAR *pName, IUnknown *pUnknown, HRESULT *phr) : 
	CBaseFilter(pName, pUnknown, &cs, CLSID_DSSink)
{
	pin = new InputPin(INPUT_PIN_NAME, this, &cs, phr, INPUT_PIN_NAME);
	if(pin && SUCCEEDED(*phr))
		return;
	if(pin)
	{
		delete pin;
		pin = NULL;
	}
	if(SUCCEEDED(*phr))
		*phr = E_OUTOFMEMORY;
}

Filter::~Filter()
{
	if(pin)
		delete pin;
}

CBasePin *Filter::GetPin(int Index)
{
	if(Index == 0)
		return pin;
	return NULL;
}

int Filter::GetPinCount()
{
	return NUMELMS(f_Pins);
}

HRESULT Filter::SetCallback(void *param, void (WINAPI *SampleCallback)(void *param, IMediaSample *))
{
	pin->SampleCallback = SampleCallback;
	pin->param = param;
	return 0;
}

AMOVIESETUP_FILTER *Filter::GetSetupData()
{
	return &f_Filter;
}

HRESULT Filter::NonDelegatingQueryInterface(const IID &riid, void **ppv)
{
	if( riid == IID_IDSSink)
		return GetInterface(static_cast<IDSSink*>(this), ppv);
	else
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

InputPin::InputPin(TCHAR *pClassName, Filter *pFilter, CCritSec *pcs, HRESULT *phr, TCHAR *pPinName) :
	CBaseInputPin(pClassName, pFilter, pcs, phr, pPinName)
{
	SampleCallback = 0;
	param = 0;
}

InputPin::~InputPin()
{
}

HRESULT InputPin::CheckMediaType(const CMediaType* pMediaType)
{
	if(!pMediaType)
	{
		ASSERT(false);
		return E_POINTER;
	}
	return S_OK;
}

HRESULT InputPin::Receive(IMediaSample* pSample)
{
	// Note CBaseInputPin::Receive validates pSample
	HRESULT hr = CBaseInputPin::Receive(pSample);
	if(FAILED(hr))
		return hr;
	if(SampleCallback)
		SampleCallback(param, pSample);
	return S_OK;
}
