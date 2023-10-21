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

#define IMG_N 8
#define IMG_D 384

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
  int index = 0;
  while (1) {
    uint8_t* sample_img = NULL;
    switch (index) {
      case 0:
        sample_img = stbi_load_from_memory(__01_bmp, sizeof(__01_bmp), &x, &y, &n, 3);
        break;
      case 1:
        sample_img = stbi_load_from_memory(__02_bmp, sizeof(__02_bmp), &x, &y, &n, 3);
        break;
      case 2:
        sample_img = stbi_load_from_memory(__03_bmp, sizeof(__03_bmp), &x, &y, &n, 3);
        break;
      case 3:
        sample_img = stbi_load_from_memory(__04_bmp, sizeof(__04_bmp), &x, &y, &n, 3);
        break;
      case 4:
        sample_img = stbi_load_from_memory(__05_bmp, sizeof(__05_bmp), &x, &y, &n, 3);
        break;
      case 5:
        sample_img = stbi_load_from_memory(__06_bmp, sizeof(__06_bmp), &x, &y, &n, 3);
        break;
      case 6:
        sample_img = stbi_load_from_memory(__07_bmp, sizeof(__07_bmp), &x, &y, &n, 3);
        break;
      case 7:
        sample_img = stbi_load_from_memory(__08_bmp, sizeof(__08_bmp), &x, &y, &n, 3);
        break;
      default:
        break;
    }
    if (!sample_img) continue;

    memset(s_frameBuffer, 0x00, sizeof(s_frameBuffer));
    int img_pos = 0;
    for (size_t i = 0; i < APP_IMG_HEIGHT; i++)
      for (size_t j = 0; j < APP_IMG_WIDTH; j++) {
        size_t pos = 3 * (i * APP_IMG_WIDTH + j);
        if (j >= x || i >= y) {
          s_frameBuffer[pos] = 0;
          s_frameBuffer[pos + 1] = 0;
          s_frameBuffer[pos + 2] = 0;
        } else {
          s_frameBuffer[pos] = sample_img[img_pos + 2];
          s_frameBuffer[pos + 1] = sample_img[img_pos + 1];
          s_frameBuffer[pos + 2] = sample_img[img_pos];
          img_pos += 3;
        }
      }

    // /* Expected tensor dimensions: [batches, height, width, channels] */
    // if (IMAGE_GetImage(inputData, inputDims.data[2], inputDims.data[1], inputDims.data[3]) != kStatus_Success) {
    //   PRINTF("Failed retrieving input image" EOL);
    //   for (;;) {
    //   }
    // }

    // memcpy(sample_img, inputData, inputDims.data[2] * inputDims.data[1] * inputDims.data[3]);

    // slding 128x128 window on 320x320 image from sample_img
    // step 64x64
    // fill on the frame buffer beside the image
    static uint8_t window_img[128 * 128 * 3];
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
        int img_pos = 0;
        for (int k = 0; k < 128; k++) {
          for (int l = 0; l < 128; l++) {
            int pos = 3 * ((k + i * 64) * IMG_D + l + j * 64);
            window_img[img_pos] = sample_img[pos + 2];
            window_img[img_pos + 1] = sample_img[pos + 1];
            window_img[img_pos + 2] = sample_img[pos];

            int buf_pos = 3 * (k * APP_IMG_WIDTH + l);
            s_frameBuffer[buf_pos] = sample_img[pos + 2];
            s_frameBuffer[buf_pos + 1] = sample_img[pos + 1];
            s_frameBuffer[buf_pos + 2] = sample_img[pos];

            img_pos += 3;
          }
        }

        ELCDIF_SetNextBufferAddr(APP_ELCDIF, (uint32_t)s_frameBuffer);
        memcpy(inputData, window_img, inputDims.data[2] * inputDims.data[1] * inputDims.data[3]);

        MODEL_ConvertInput(inputData, &inputDims, inputType);

        auto startTime = TIMER_GetTimeInUS();
        MODEL_RunInference();
        auto endTime = TIMER_GetTimeInUS();

        int result_index;
        int confidence;

        MODEL_ProcessOutput(outputData, &outputDims, outputType, endTime - startTime, &result_index, &confidence);

        PRINTF("::: {\"id\": %d, \"label\": %d, \"confident\": %d}\n", index >> 1, result_index, confidence);

        uint32_t now = TIMER_GetTimeInUS();
        while (TIMER_GetTimeInUS() - now < 10000)
          ;
      }
    }

    stbi_image_free(sample_img);
    index = (index + 1) % IMG_N;
  }
}
