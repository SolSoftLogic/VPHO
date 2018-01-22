/*****************************************************************************
 * bdagraph.h : DirectShow BDA graph builder header for vlc
 *****************************************************************************
 * Copyright ( C ) 2007 the VideoLAN team
 *
 * Author: Ken Self <kens@campoz.fslife.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#define _WIN32_DCOM


#include <queue>
using namespace std;
#include <dshow.h>
#include <comcat.h>
#include "bdadefs.h"
#include <qedit.h>

class QFFMPEG;
class QFFFIFO;
class DXCAPTUREFRAMEBASE;

/* The main class for building the filter graph */
class BDAGraph : public ISampleGrabberCB
{
public:
    BDAGraph();
    virtual ~BDAGraph();

    int SubmitATSCTuneRequest();
    int SubmitDVBTTuneRequest();
    int SubmitDVBCTuneRequest();
    int SubmitDVBSTuneRequest();
	int SelectDevice(TCHAR *device);
	void SetResolution(int xres, int yres) { this->xres = xres; this->yres = yres; }
	void SetFourcc(unsigned fourcc) { this->fourcc = fourcc; }
	void SetCaptureCallback(DXCAPTUREFRAMEBASE *obj) { cap = obj; }
	int SetFrequency(int freq) { frequency = freq; programid = 0; return 0; }
	int SetProgramId(int id);
    long GetBufferSize();
    long ReadBuffer( long* l_buffer_len, BYTE* p_buff );
	int ProgramsList(char *list, int size);

private:
	friend void AcquireThread(void *obj);
	void AcquireThread();
    /* ISampleGrabberCB methods */
    STDMETHODIMP_( ULONG ) AddRef( ) { return 1; }
    STDMETHODIMP_( ULONG ) Release( ) { return 2; }
    STDMETHODIMP QueryInterface( REFIID riid, void** p_p_object )
        {return E_NOTIMPL;  }
    STDMETHODIMP SampleCB( double d_time, IMediaSample* p_sample );
    STDMETHODIMP BufferCB( double d_time, BYTE* p_buffer, long l_buffer_len );

    CLSID     guid_network_type;
    long      l_tuner_used;        /* Index of the Tuning Device */
    /* registration number for the RunningObjectTable */
    DWORD     d_graph_register;

	// QFFMPEG support
	DXCAPTUREFRAMEBASE *cap;
	int frequency;
	QFFMPEG *qffmpeg;
	QFFFIFO *fifo;
	unsigned fourcc;
	int xres, yres;
	int programid;
	bool threadrunning, stopthread;
	char bdadevice[100];
	CRITICAL_SECTION cs;

    queue<IMediaSample*> queue_sample;
    queue<IMediaSample*> queue_buffer;
    BOOL b_ready;

    IMediaControl*  p_media_control;
    IGraphBuilder*  p_filter_graph;
    ITuningSpace*   p_tuning_space;
    ITuneRequest*   p_tune_request;

    ICreateDevEnum* p_system_dev_enum;
    IBaseFilter*    p_network_provider;
    IScanningTuner* p_scanning_tuner;
    IBaseFilter*    p_tuner_device;
    IBaseFilter*    p_capture_device;
    IBaseFilter*    p_sample_grabber;
    IBaseFilter*    p_mpeg_demux;
    IBaseFilter*    p_transport_info;
    ISampleGrabber* p_grabber;

    HRESULT CreateTuneRequest( );
    HRESULT Build( );
    HRESULT FindFilter( REFCLSID clsid, long* i_moniker_used,
        IBaseFilter* p_upstream, IBaseFilter** p_p_downstream, const char *name = 0);
    HRESULT Connect( IBaseFilter* p_filter_upstream,
        IBaseFilter* p_filter_downstream );
    HRESULT Start( );
    HRESULT Destroy( );
    HRESULT Register( );
    void Deregister( );
};
