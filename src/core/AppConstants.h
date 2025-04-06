#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

namespace Res {
    namespace Shader {
        constexpr const char* CHUNK_DEPTH = "res/shaders/chunkDepthShader";
        constexpr const char* CHUNK_SSAO_GBUFFER = "res/shaders/chunkSSAO_GBufferShader";
        constexpr const char* CHUNK = "res/shaders/chunkShader";
        constexpr const char* SIMPLE = "res/shaders/simpleShader";
        constexpr const char* DEPTH = "res/shaders/depthShader";
        constexpr const char* LINE = "res/shaders/lineShader";
        constexpr const char* SSAO_PASS = "res/shaders/SSAO_PassShader";
        constexpr const char* SSAO_BLUR = "res/shaders/SSAO_BlurShader";
    }

    namespace Texture {
        constexpr const char* BLOCK_TEX_ATLAS = "res/textures/blockTexAtlas.png";
        constexpr const char* TESTBLOCK_TEXTURE = "res/textures/testTexture.png";
    }

    namespace Font {
        constexpr const char* PROGGY_CLEAN = "res/fonts/ProggyClean.ttf";
        constexpr const char* PROGGY_TINY = "res/fonts/ProggyTiny.ttf";
    }

    namespace Model {
        constexpr const char* TEST_UNIT_BLOCK = "res/models/testUnitBlock.obj";
    }
};

#endif