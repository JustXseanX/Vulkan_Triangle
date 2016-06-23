//-------------------------------------------------------------------------------------------------
// File : SimpleVS.glsl
// Desc : Sample Vertex Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//-------------------------------------------------------------------------------------------------
// Input Definitions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) in vec3 InputPosition;     // 位置座標.
layout(location = 1) in vec2 InputTexCoord;     // テクスチャ座標.
layout(location = 2) in vec4 InputColor;        // 頂点カラー.

//-------------------------------------------------------------------------------------------------
// Output Defintions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) out vec2 OutputTexCoord;   // テクスチャ座標.
layout(location = 1) out vec4 OutputColor;      // 頂点カラー.
out gl_PerVertex 
{
    vec4 gl_Position;   // 位置座標.
};

//-------------------------------------------------------------------------------------------------
//      頂点シェーダメインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
void main()
{
    vec4 localPos = vec4(InputPosition, 1.0f);

    gl_Position    = localPos;
    OutputTexCoord = InputTexCoord;
    OutputColor    = InputColor;
}
