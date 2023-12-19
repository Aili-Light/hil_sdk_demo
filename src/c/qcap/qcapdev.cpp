#include "qcapdev.h"

QCapDev::QCapDev(int width, int height, int channel, int frame_rate, uint32_t flags)
{
    m_hVideoDevice = NULL;

    m_nVideoWidth = width;

    m_nVideoHeight = height;

    m_bVideoIsInterleaved = 0;

    m_dVideoFrameRate = frame_rate;

    m_nAudioChannels = 0;

    m_nAudioBitsPerSample = 0;

    m_nAudioSampleFrequency = 0;

    ch_id = channel;

    m_Flags = flags;

    HwInitialize();
}

QCapDev::~QCapDev()
{
    HwUninitialize();
}

void QCapDev::setQcapCallbackNoSignal(cb_func_signal callback)
{
    ULONG i = ch_id;

    QCAP_REGISTER_NO_SIGNAL_DETECTED_CALLBACK(m_hVideoDevice, callback, (PVOID)i);
}

void QCapDev::setQcapCallbackSignalRemoved(cb_func_signal callback)
{
    ULONG i = ch_id;

    QCAP_REGISTER_SIGNAL_REMOVED_CALLBACK(m_hVideoDevice, callback, (PVOID)i);
}

void QCapDev::setQcapCallbackFormatChanged(cb_func_format callback)
{
    ULONG i = ch_id;

    QCAP_REGISTER_FORMAT_CHANGED_CALLBACK(m_hVideoDevice, callback, (PVOID)i);
}

void QCapDev::setQcapCallbackVideoPreview(cb_func_preview callback)
{
    ULONG i = ch_id;

    QCAP_REGISTER_VIDEO_PREVIEW_CALLBACK(m_hVideoDevice, callback, (PVOID)i);
}

void QCapDev::HwInitialize()
{
    char CapDevName[16] = "SC0720 PCI";
    ULONG i = ch_id;

    if (QCAP_CREATE(CapDevName, ch_id, 0, &m_hVideoDevice, TRUE) != QCAP_RS_SUCCESSFUL)
    {
        printf("Open Device [%d] FAILED!\n", ch_id);
        return;
    }

    printf("Open Device Success!\n");

    QCAP_SET_VIDEO_INPUT(m_hVideoDevice, QCAP_INPUT_TYPE_HDMI);

    // QCAP_REGISTER_NO_SIGNAL_DETECTED_CALLBACK(m_hVideoDevice, on_process_no_signal_detected, (PVOID)i);

    // QCAP_REGISTER_SIGNAL_REMOVED_CALLBACK(m_hVideoDevice, on_process_signal_removed, (PVOID)i);

    // QCAP_REGISTER_FORMAT_CHANGED_CALLBACK(m_hVideoDevice, on_process_format_changed, (PVOID)i);

    // QCAP_REGISTER_VIDEO_PREVIEW_CALLBACK(m_hVideoDevice, on_video_preview_callback, (PVOID)i);

    // QCAP_REGISTER_AUDIO_PREVIEW_CALLBACK( m_hVideoDevice[i], on_audio_preview_callback, (PVOID)i );

    //        QCAP_SET_VIDEO_DEFAULT_OUTPUT_FORMAT( m_hVideoDevice[i], QCAP_COLORSPACE_TYEP_YUY2, 0, 0, 0, 0);
    QCAP_SET_VIDEO_DEFAULT_OUTPUT_FORMAT(m_hVideoDevice, QCAP_COLORSPACE_TYPE_YUY2, m_nVideoWidth, m_nVideoHeight, 0, m_dVideoFrameRate);

    printf("Set callback functions!\n");

    QCAP_RUN(m_hVideoDevice);

    printf("Start to run!\n");
}

void QCapDev::HwUninitialize()
{
    // DESTROY CAPTURE DEVICE

    if (m_hVideoDevice)
    {

        QCAP_STOP(m_hVideoDevice);

        QCAP_DESTROY(m_hVideoDevice);

        m_hVideoDevice = NULL;
    }
}
