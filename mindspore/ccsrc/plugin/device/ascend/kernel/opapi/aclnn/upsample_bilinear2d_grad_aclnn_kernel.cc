/**
 * Copyright 2024 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "plugin/device/ascend/kernel/opapi/aclnn/upsample_bilinear2d_grad_aclnn_kernel.h"

#include <algorithm>
#include <vector>
#include <map>
#include <memory>
#include <tuple>
#include <functional>

#include "ir/tensor.h"
#include "mindapi/base/types.h"
#include "transform/acl_ir/acl_helper.h"
#include "transform/acl_ir/op_api_convert.h"
#include "abstract/ops/primitive_infer_map.h"

namespace mindspore {
namespace kernel {
namespace {
std::tuple<std::vector<int64_t>, std::vector<int64_t>, std::tuple<double, double>, bool> UpsampleBilinear2DGradGenerate(
  const std::vector<KernelTensor *> &inputs, const std::vector<KernelTensor *> &outputs) {
  auto input_size = inputs[kIndex1]->GetValueWithCheck<std::vector<int64_t>>();

  auto grad_out_shape = inputs[kIndex0]->GetShapeVector();
  std::vector<int64_t> output_size{grad_out_shape.begin() + kIndex2, grad_out_shape.end()};

  std::vector<pyfloat> scales{0., 0.};
  if (inputs[kIndex3]->GetType()->type_id() != kMetaTypeNone) {
    MS_EXCEPTION(RuntimeError) << "For UpsampleBilinear2DGrad, scale_factors is not supported now.";
    scales = inputs[kIndex3]->GetValueWithCheck<std::vector<pyfloat>>();
  }

  bool align_corners = inputs[kIndex4]->GetValueWithCheck<bool>();

  double scales_h = scales[0];
  double scales_w = scales[1];

  return std::make_tuple(std::move(input_size), std::move(output_size), std::make_tuple(scales_h, scales_w),
                         align_corners);
}
}  // namespace

void UpsampleBilinear2DGradAscend::GetWorkSpaceInfo(const std::vector<KernelTensor *> &inputs,
                                                    const std::vector<KernelTensor *> &outputs) {
  auto params = UpsampleBilinear2DGradGenerate(inputs, outputs);
  const auto &input_size = std::get<0>(params);
  const auto &output_size = std::get<1>(params);
  auto [scales_h, scales_w] = std::get<2>(params);
  auto align_corners = std::get<3>(params);
  GetWorkspaceForResize(inputs[0], output_size, input_size, align_corners, scales_h, scales_w, outputs[0]);
}

bool UpsampleBilinear2DGradAscend::Launch(const std::vector<KernelTensor *> &inputs,
                                          const std::vector<KernelTensor *> &workspace,
                                          const std::vector<KernelTensor *> &outputs, void *stream_ptr) {
  MS_EXCEPTION_IF_NULL(stream_ptr);

  auto params = UpsampleBilinear2DGradGenerate(inputs, outputs);
  const auto &input_size = std::get<0>(params);
  const auto &output_size = std::get<1>(params);
  auto [scales_h, scales_w] = std::get<2>(params);
  auto align_corners = std::get<3>(params);

  ParseGenExecutor(GEN_EXECUTOR_BOOST(op_type_, hash_id_, inputs[0], output_size, input_size, align_corners, scales_h,
                                      scales_w, outputs[0]));
  RunOp(stream_ptr, workspace);
  return true;
}

MS_ACLNN_KERNEL_FACTORY_REG(UpsampleBilinear2DGrad, UpsampleBilinear2DGradAscend);
}  // namespace kernel
}  // namespace mindspore
