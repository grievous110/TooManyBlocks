#include "ShaderBuilder.h"

Future<Shader> build(const Future<CPUShader>& cpuShader) {
    Future<Shader> shaderFuture(
        [cpuShader]() {
            const CPUShader& cpu = cpuShader.value();
            return Shader::create(cpu.vertexShader, cpu.fragmentShader);
        },
        DEFAULT_TASKCONTEXT,
        Executor::Main
    );

    return shaderFuture.start();
}

Future<TransformFeedbackShader> buildTFShader(
    const Future<CPUShader>& cpuShader,
    const std::vector<std::string>& varyings
) {
    Future<TransformFeedbackShader> tfFuture(
        [cpuShader, varyings]() {
            const CPUShader& cpu = cpuShader.value();
            return TransformFeedbackShader::create(cpu.vertexShader, varyings);
        },
        DEFAULT_TASKCONTEXT,
        Executor::Main
    );

    return tfFuture.start();
}