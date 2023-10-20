/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "demo_config.h"
#include "demo_info.h"
#include "fsl_debug_console.h"
#include "fsl_elcdif.h"
#include "image.h"
#include "image_data.h"
#include "image_utils.h"
#include "lcd.h"
#include "model.h"
#include "output_postproc.h"
#include "timer.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#include "sample_imgs.h"
#include "stb_image.h"
/* LCD Driver */

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_frameBuffer[APP_IMG_HEIGHT * APP_IMG_WIDTH * 3], FRAME_BUFFER_ALIGN);
static volatile bool s_frameDone = false;

void APP_ELCDIF_Init(void) {
  const elcdif_rgb_mode_config_t config = {
      .panelWidth = APP_IMG_WIDTH,
      .panelHeight = APP_IMG_HEIGHT,
      .hsw = APP_HSW,
      .hfp = APP_HFP,
      .hbp = APP_HBP,
      .vsw = APP_VSW,
      .vfp = APP_VFP,
      .vbp = APP_VBP,
      .polarityFlags = APP_POL_FLAGS,
      .bufferAddr = (uint32_t)s_frameBuffer,
      .pixelFormat = kELCDIF_PixelFormatRGB888,
      .dataBus = kELCDIF_DataBus16Bit,
  };

#if (defined(APP_ELCDIF_HAS_DISPLAY_INTERFACE) && APP_ELCDIF_HAS_DISPLAY_INTERFACE)
  BOARD_InitDisplayInterface();
#endif
  ELCDIF_RgbModeInit(APP_ELCDIF, &config);
}

void APP_LCDIF_IRQHandler(void) {
  uint32_t intStatus;

  intStatus = ELCDIF_GetInterruptStatus(APP_ELCDIF);

  ELCDIF_ClearInterruptStatus(APP_ELCDIF, intStatus);

  if (intStatus & kELCDIF_CurFrameDone) {
    s_frameDone = true;
  }
  SDK_ISR_EXIT_BARRIER;
}

void LCDIF_IRQHandler(void) {
  APP_LCDIF_IRQHandler();
  __DSB();
}

/* LCD Driver */

int main(void) {
  BOARD_Init();
  TIMER_Init();
  APP_ELCDIF_Init();

  DEMO_PrintInfo();

  memset(s_frameBuffer, 0x00, sizeof(s_frameBuffer));

  // ELCDIF_EnableInterrupts(APP_ELCDIF, kELCDIF_CurFrameDoneInterruptEnable);
  ELCDIF_RgbModeStart(APP_ELCDIF);

  if (MODEL_Init() != kStatus_Success) {
    PRINTF("Failed initializing model" EOL);
    for (;;) {
    }
  }

  tensor_dims_t inputDims;
  tensor_type_t inputType;
  uint8_t* inputData = MODEL_GetInputTensorData(&inputDims, &inputType);

  tensor_dims_t outputDims;
  tensor_type_t outputType;
  uint8_t* outputData = MODEL_GetOutputTensorData(&outputDims, &outputType);

  int x, y, n;
  uint8_t* sample_imgs_1 = stbi_load_from_memory(__01_bmp, sizeof(__01_bmp), &x, &y, &n, 3);

  if (sample_imgs_1)
    for (size_t i = 0; i < APP_IMG_HEIGHT; i++)
      for (size_t j = 0; j < APP_IMG_WIDTH; j++) {
        size_t pos = 3 * (i * APP_IMG_WIDTH + j);
        s_frameBuffer[pos] = sample_imgs_1[pos + 2];
        s_frameBuffer[pos + 1] = sample_imgs_1[pos + 1];
        s_frameBuffer[pos + 2] = sample_imgs_1[pos];
      }
  // memcpy(s_frameBuffer, sample_imgs_1, x * y * n);

  while (1) {
    /* Expected tensor dimensions: [batches, height, width, channels] */
    if (IMAGE_GetImage(inputData, inputDims.data[2], inputDims.data[1], inputDims.data[3]) != kStatus_Success) {
      PRINTF("Failed retrieving input image" EOL);
      for (;;) {
      }
    }

    MODEL_ConvertInput(inputData, &inputDims, inputType);

    auto startTime = TIMER_GetTimeInUS();
    MODEL_RunInference();
    auto endTime = TIMER_GetTimeInUS();

    MODEL_ProcessOutput(outputData, &outputDims, outputType, endTime - startTime);

    ELCDIF_SetNextBufferAddr(APP_ELCDIF, (uint32_t)s_frameBuffer);
  }
}
