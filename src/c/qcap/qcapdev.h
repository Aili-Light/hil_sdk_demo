#ifndef __QCAPDEV_H__
#define __QCAPDEV_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/qcap.h"
#include "include/qcap.windef.h"
#include "include/qcap.linux.h"

typedef QRETURN(*cb_func_signal)(void*, ULONG, ULONG, void*);
typedef QRETURN(*cb_func_format)(void*, ULONG, ULONG, ULONG, ULONG, BOOL, double, ULONG, ULONG, ULONG, void*);
typedef QRETURN(*cb_func_preview)(void*, double, BYTE*, ULONG, void*);

class QCapDev
{
public:
	enum Flags
	{
		Default        = (1 << 0),			
		WriteFrame     = (1 << 1),	
		Reserved       = (1 << 2),	
	};

    QCapDev   (int width=0, int height=0, int channel=0, int frame_rate=30, uint32_t flags=Default);
    ~QCapDev  ();

    /* CALLBACK */
    void setQcapCallbackNoSignal(cb_func_signal callback);

    void setQcapCallbackSignalRemoved(cb_func_signal callback);

    void setQcapCallbackFormatChanged(cb_func_format callback);

    void setQcapCallbackVideoPreview(cb_func_preview callback);

    /* DEVICE INFO */

    PVOID     m_hVideoDevice;

    /* VIDEO INFO */

    ULONG     m_nVideoWidth;
    
    ULONG     m_nVideoHeight;

    bool      m_bVideoIsInterleaved;

    double    m_dVideoFrameRate;

    /* ADUIO INFO */

    ULONG     m_nAudioChannels;
    
    ULONG     m_nAudioBitsPerSample;
    
    ULONG     m_nAudioSampleFrequency;

    /* CONFIGURATION */
    uint32_t  m_Flags;
    int       ch_id;

private:
    void HwInitialize();
    void HwUninitialize();
};



#endif