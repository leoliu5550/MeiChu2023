/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

tflite::MicroOpResolver &MODEL_GetOpsResolver() {
  static tflite::MicroMutableOpResolver<14> s_microOpResolver;
  s_microOpResolver.AddConv2D();
  s_microOpResolver.AddDepthwiseConv2D();
  s_microOpResolver.AddFullyConnected();
  s_microOpResolver.AddSoftmax();
  s_microOpResolver.AddReshape();
  s_microOpResolver.AddMaxPool2D();
  s_microOpResolver.AddRelu();
  s_microOpResolver.AddConcatenation();
  s_microOpResolver.AddAdd();
  s_microOpResolver.AddLeakyRelu();
  s_microOpResolver.AddPad();
  s_microOpResolver.AddResizeNearestNeighbor();
  s_microOpResolver.AddAveragePool2D();
  s_microOpResolver.AddDequantize();

  return s_microOpResolver;
}
