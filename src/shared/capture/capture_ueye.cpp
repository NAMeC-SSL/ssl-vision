#include <iostream>
#include <ueye.h>
#include "capture_ueye.h"

CaptureUeye::CaptureUeye(VarList * _settings, int default_camera_id, QObject * parent)
: QObject(parent), CaptureInterface(_settings)
{
    is_capturing = false;
    capture_width = 1280;
    capture_height = 1024;
    current_frame = nullptr;

    // Camera ID
    v_cam_id = new VarInt("Cam ID", default_camera_id+1);
    _settings->addChild(v_cam_id);

    // Capture dimensions
    v_width = new VarInt("Width", 1280);
    _settings->addChild(v_width);
    v_height = new VarInt("Height", 1024);
    _settings->addChild(v_height);
    v_x = new VarInt("X offset", 0);
    _settings->addChild(v_x);
    v_y = new VarInt("Y offset", 0);
    _settings->addChild(v_y);

    // Exposure time
    v_exposure = new VarDouble("Exposure (ms)", 16.35);
    _settings->addChild(v_exposure);

    // Frames per second
    v_fps = new VarDouble("FPS", 60.01);
    _settings->addChild(v_fps);

    // Gains
    v_master_gain = new VarInt("Master gain", 0);
    _settings->addChild(v_master_gain);

    v_red_gain = new VarInt("Red gain", 0);
    _settings->addChild(v_red_gain);

    v_green_gain = new VarInt("Green gain", 0);
    _settings->addChild(v_green_gain);

    v_blue_gain = new VarInt("Blue gain", 64);
    _settings->addChild(v_blue_gain);

    // Edge enhancement value
    v_edge_enhancement = new VarInt("Edge enhancement", 15);
    _settings->addChild(v_edge_enhancement);
}

CaptureUeye::~CaptureUeye()
{
    if (is_capturing) {
        stopCapture();
    }
}

RawImage CaptureUeye::getFrame()
{
    char *dummy;
    char *old_frame = current_frame;
    is_GetActSeqBuf(hCam, &mem_id, &dummy, &current_frame);
    mem_id -= 1;
    if (mem_id == 0) mem_id = 8;

    if (old_frame == current_frame) {
        mutex.lock();
        is_WaitEvent(hCam, IS_SET_EVENT_FRAME, 100);
        mutex.unlock();
        is_GetActSeqBuf(hCam, &mem_id, &dummy, &current_frame);
    }

    RawImage result;
    result.setWidth(capture_width);
    result.setHeight(capture_height);
    result.setData((unsigned char*)current_frame);
    result.setColorFormat(COLOR_YUV422_UYVY);

    struct timeval tv;
    gettimeofday(&tv,NULL);
    result.setTime((double)tv.tv_sec + tv.tv_usec*(1.0E-6));

    // Camera capture time
//  UEYEIMAGEINFO info;
//  is_GetImageInfo(hCam, mem_id, &info, sizeof(UEYEIMAGEINFO));
//  result.setTime(info.u64TimestampDevice/10000000.0);
    // printf("Time: %f\n", result.getTime());

    return result;
}

bool CaptureUeye::isCapturing()
{
    return is_capturing;
}

void CaptureUeye::releaseFrame()
{
}

bool CaptureUeye::startCapture()
{
    mutex.lock();
    hCam = v_cam_id->getInt();
    std::cout << "uEye: Starting capturing! index: " << hCam << std::endl;

    is_capturing = false;
    if (is_InitCamera(&hCam, NULL) != IS_SUCCESS) {
        mutex.unlock();
        return false;
    }

    // Getting dimensions
    capture_width = v_width->getInt();
    capture_height = v_height->getInt();

    // Searching for format
    uint32_t entries;
    is_ImageFormat(hCam, IMGFRMT_CMD_GET_NUM_ENTRIES, &entries, sizeof(entries));

    char formats[sizeof(IMAGE_FORMAT_LIST) + (entries-1)*sizeof(IMAGE_FORMAT_INFO)];
    IMAGE_FORMAT_LIST *formatList = (IMAGE_FORMAT_LIST*)formats;
    formatList->nNumListElements = entries;
    formatList->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);
    is_ImageFormat(hCam, IMGFRMT_CMD_GET_LIST, formats, sizeof(formats));
    bool found = false;
    for (size_t k=0; k<entries; k++) {
        IMAGE_FORMAT_INFO *info = &formatList->FormatInfo[k];

        // printf("w: %d, h: %d\n", info->nWidth, info->nHeight);
        if (info->nWidth == 1280 && info->nHeight == 1024) {
            is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &info->nFormatID, sizeof(info->nFormatID));
            found = true;
        }
    }

    if (!found) {
        std::cerr << "Unsupported resolution: " << capture_width << "x" << capture_height << std::endl;
    }

    IS_RECT rectAOI;
    rectAOI.s32X = v_x->getInt();
    rectAOI.s32Y = v_y->getInt();
    rectAOI.s32Width = capture_width;
    rectAOI.s32Height = capture_height;
    is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI));

    // Image memory allocation
    is_ClearSequence(hCam);
    buffers.clear();
    for (int k=0; k<8; k++) {
        char *mem;
        int memId;
        is_AllocImageMem(hCam, capture_width, capture_height, 16, &mem, &memId);
        is_AddToSequence(hCam, mem, memId);
        buffers[memId] = mem;
    }
    last_mem_id = 1;

    // Exposure
    double exposure = v_exposure->getDouble();
    is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&exposure, sizeof(exposure));

    // White balance
    is_SetHardwareGain(hCam, v_master_gain->getInt(), v_red_gain->getInt(), v_green_gain->getInt(), v_blue_gain->getInt());

    // Enabling anti-flicker
    double flicker = ANTIFLCK_MODE_SENS_50_FIXED;
    is_SetAutoParameter(hCam, IS_SET_ANTI_FLICKER_MODE, &flicker, NULL);

    // Setting framerate
    is_SetColorMode(hCam, IS_CM_UYVY_PACKED);
    double fps = v_fps->getDouble(), newFps = v_fps->getDouble();
    is_SetFrameRate(hCam, fps, &newFps);

    // Event frame event enabled
    is_EnableEvent(hCam, IS_SET_EVENT_FRAME);

    // Video capture
    is_CaptureVideo(hCam, IS_WAIT);

    // Edge enhancement
    UINT nEdgeEnhancement = v_edge_enhancement->getInt();
    printf("EdgeEnhancement: %d\n", is_EdgeEnhancement(hCam, IS_EDGE_ENHANCEMENT_CMD_SET, (void*)&nEdgeEnhancement, sizeof(nEdgeEnhancement)));

    is_capturing = true;
    mutex.unlock();
    return true;
}

bool CaptureUeye::stopCapture()
{
    is_capturing = false;

    mutex.lock();
    is_ClearSequence(hCam);

    for (std::map<int, char*>::iterator it = buffers.begin();
        it != buffers.end(); it++) {
        is_FreeImageMem(hCam, it->second, it->first);
    }
    is_ExitCamera(hCam);
    current_frame = NULL;
    mutex.unlock();

    return true;
}

bool CaptureUeye::resetBus()
{
    return true;
}

void CaptureUeye::readAllParameterValues()
{
}

string CaptureUeye::getCaptureMethodName() const
{
    return "uEye";
}

bool CaptureUeye::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
    target.allocate(COLOR_YUV422_UYVY, src.getWidth(), src.getHeight());
    memcpy(target.getData(),src.getData(),src.getNumBytes());

    return true;
}
