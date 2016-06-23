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
layout(location = 0) in vec3 InputPosition;     // �ʒu���W.
layout(location = 1) in vec2 InputTexCoord;     // �e�N�X�`�����W.
layout(location = 2) in vec4 InputColor;        // ���_�J���[.

//-------------------------------------------------------------------------------------------------
// Output Defintions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) out vec2 OutputTexCoord;   // �e�N�X�`�����W.
layout(location = 1) out vec4 OutputColor;      // ���_�J���[.
out gl_PerVertex 
{
    vec4 gl_Position;   // �ʒu���W.
};

//-------------------------------------------------------------------------------------------------
//      ���_�V�F�[�_���C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
void main()
{
    vec4 localPos = vec4(InputPosition, 1.0f);

    gl_Position    = localPos;
    OutputTexCoord = InputTexCoord;
    OutputColor    = InputColor;
}
