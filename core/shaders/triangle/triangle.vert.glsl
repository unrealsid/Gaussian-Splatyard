#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 fragColor;

//layout(buffer_reference, std430) readonly buffer MatrixData
//{
//	mat4 projection;
//	mat4 view;
//};
//
//layout(buffer_reference, scalar) buffer ModelTransform
//{
//	mat4 model_transform;
//};
//
//layout(push_constant) uniform PushConstants
//{
//	MatrixData matrix_adddress;
//	ModelTransform model_transform_address;
//} pc;

void main()
{
//	MatrixData matrices = MatrixData(pc.matrix_adddress);
//	ModelTransform model_transform = ModelTransform(pc.model_transform_address);

	vec4 position = vec4(inPosition, 0.0, 1.0);
//	gl_Position = matrices.projection * matrices.view * position;
	gl_Position = position;
	gl_PointSize = 1.0;

	fragColor = inColor;
}