#ifndef TOOMANYBLOCKS_AUDIOPLAYBACK_H
#define TOOMANYBLOCKS_AUDIOPLAYBACK_H

#include <miniaudio.h>

void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

#endif