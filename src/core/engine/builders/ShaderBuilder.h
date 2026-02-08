#ifndef TOOMANYBLOCKS_SHADERBUILDER_H
#define TOOMANYBLOCKS_SHADERBUILDER_H

#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/TransformFeedbackShader.h"
#include "engine/resource/cpu/CPUShader.h"
#include "threading/Future.h"

Future<Shader> build(const Future<CPUShader>& cpuShader);

Future<TransformFeedbackShader> buildTFShader(const Future<CPUShader>& cpuShader, const std::vector<std::string>& varyings);

#endif
