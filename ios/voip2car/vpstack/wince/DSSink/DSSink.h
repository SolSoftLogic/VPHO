#pragma once

#include <streams.h>
#include <atlbase.h>
#include <initguid.h>

// {9360C597-CB8C-4af1-B0D2-B85ABAAFE211}
DEFINE_GUID(CLSID_DSSink, 0x9360c597, 0xcb8c, 0x4af1, 0xb0, 0xd2, 0xb8, 0x5a, 0xba, 0xaf, 0xe2, 0x11);
// {914EBC2C-E4D6-40a0-AD01-58E047E3DB13}
DEFINE_GUID(IID_IDSSink, 0x914ebc2c, 0xe4d6, 0x40a0, 0xad, 0x1, 0x58, 0xe0, 0x47, 0xe3, 0xdb, 0x13);

class __declspec( uuid("914EBC2C-E4D6-40a0-AD01-58E047E3DB13") ) IDSSink : public IUnknown
{
public:
	STDMETHOD(SetCallback)(void *param, void (WINAPI *SampleCallback)(void *param, IMediaSample *sample)) PURE;
};
