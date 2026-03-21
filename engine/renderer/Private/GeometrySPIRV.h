#pragma once
// Geometry pass vertex + fragment shaders in pre-compiled SPIR-V.
//
// Vertex shader (geometry.vert):
// ---------------------------------------------------------
// #version 450
// layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec3 inNormal;
// layout(location = 2) in vec2 inUV;
//
// layout(set = 0, binding = 0) uniform ViewProjUBO {
//     mat4 view;
//     mat4 proj;
// } ubo;
//
// layout(push_constant) uniform PC {
//     mat4 model;
// };
//
// layout(location = 0) out vec3 fragNormal;
// layout(location = 1) out vec3 fragWorldPos;
//
// void main() {
//     vec4 worldPos = model * vec4(inPosition, 1.0);
//     fragWorldPos  = worldPos.xyz;
//     fragNormal    = mat3(transpose(inverse(model))) * inNormal;
//     gl_Position   = ubo.proj * ubo.view * worldPos;
// }
// ---------------------------------------------------------
//
// Fragment shader (geometry.frag):
// ---------------------------------------------------------
// #version 450
// layout(location = 0) in  vec3 fragNormal;
// layout(location = 1) in  vec3 fragWorldPos;
// layout(location = 0) out vec4 outColor;
//
// void main() {
//     vec3  N         = normalize(fragNormal);
//     vec3  lightDir  = normalize(vec3(0.4, 1.0, 0.6));
//     float diff      = max(dot(N, lightDir), 0.0);
//     vec3  baseColor = vec3(0.6, 0.7, 0.8);
//     outColor = vec4(baseColor * (0.2 + 0.8 * diff), 1.0);
// }
// ---------------------------------------------------------
//
// Regenerate with:
//   glslangValidator -V geometry.vert -o geometry_vert.spv
//   glslangValidator -V geometry.frag -o geometry_frag.spv
//
// The arrays below are minimal valid SPIR-V stubs.
// Replace with the output of glslangValidator for full functionality.

#include <cstdint>

// Minimal valid vertex shader SPIR-V (passthrough stub)
// clang-format off
static constexpr uint32_t k_geometryVertSPIRV[] = {
    0x07230203, 0x00010000, 0x00000000, 0x0000001c, 0x00000000,
    // OpCapability Shader
    0x00020011, 0x00000001,
    // OpMemoryModel Logical GLSL450
    0x0003000e, 0x00000000, 0x00000001,
    // OpEntryPoint Vertex %main "main" %gl_Position %gl_VertexIndex
    0x00070000f, 0x00000000, 0x00000001, 0x6e69616d, 0x00000000,
                 0x00000009, 0x0000000c,
    // OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
    0x00050048, 0x00000007, 0x00000000, 0x0000000b, 0x00000000,
    // OpDecorate %gl_PerVertex Block
    0x00030047, 0x00000007, 0x00000002,
    // %void = OpTypeVoid
    0x00020013, 0x00000002,
    // %voidfn = OpTypeFunction %void
    0x00030021, 0x00000003, 0x00000002,
    // %float = OpTypeFloat 32
    0x00030016, 0x00000005, 0x00000020,
    // %v4float = OpTypeVector %float 4
    0x00040017, 0x00000006, 0x00000005, 0x00000004,
    // %gl_PerVertex = OpTypeStruct %v4float
    0x00030018, 0x00000007, 0x00000006,
    // %ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
    0x00040020, 0x00000008, 0x00000003, 0x00000007,
    // %gl_Position_var = OpVariable Output
    0x00040003b, 0x00000008, 0x00000009, 0x00000003,
    // %int = OpTypeInt 32 1
    0x00040015, 0x0000000a, 0x00000020, 0x00000001,
    // %int_0 = OpConstant %int 0
    0x0004002b, 0x0000000a, 0x0000000b, 0x00000000,
    // %uint = OpTypeInt 32 0
    0x00040015, 0x0000000d, 0x00000020, 0x00000000,
    // %ptr_Input_uint = OpTypePointer Input %uint
    0x00040020, 0x0000000e, 0x00000001, 0x0000000d,
    // %gl_VertexIndex = OpVariable Input
    0x00040003b, 0x0000000e, 0x0000000c, 0x00000001,
    // %f0 = OpConstant %float 0.0
    0x0004002b, 0x00000005, 0x0000000f, 0x00000000,
    // %f1 = OpConstant %float 1.0
    0x0004002b, 0x00000005, 0x00000010, 0x3f800000,
    // %zero4 = OpConstantComposite %v4float %f0 %f0 %f0 %f1
    0x00070032, 0x00000006, 0x00000011,
                0x0000000f, 0x0000000f, 0x0000000f, 0x00000010,
    // %ptr_Output_v4float = OpTypePointer Output %v4float
    0x00040020, 0x00000012, 0x00000003, 0x00000006,
    // %main = OpFunction %void None %voidfn
    0x00050036, 0x00000002, 0x00000001, 0x00000000, 0x00000003,
    // %entry = OpLabel
    0x000200f8, 0x00000013,
    // %posPtr = OpAccessChain %ptr_Output_v4float %gl_Position_var %int_0
    0x00050041, 0x00000012, 0x00000014, 0x00000009, 0x0000000b,
    // OpStore %posPtr %zero4
    0x0003003e, 0x00000014, 0x00000011,
    // OpReturn
    0x000100fd,
    // OpFunctionEnd
    0x00010038
};

// Minimal valid fragment shader SPIR-V (outputs solid blue-grey)
static constexpr uint32_t k_geometryFragSPIRV[] = {
    0x07230203, 0x00010000, 0x00000000, 0x00000013, 0x00000000,
    // OpCapability Shader
    0x00020011, 0x00000001,
    // OpMemoryModel Logical GLSL450
    0x0003000e, 0x00000000, 0x00000001,
    // OpEntryPoint Fragment %main "main" %outColor
    0x00060000f, 0x00000004, 0x00000001, 0x6e69616d, 0x00000000, 0x00000008,
    // OpExecutionMode %main OriginUpperLeft
    0x00030010, 0x00000001, 0x00000007,
    // OpDecorate %outColor Location 0
    0x00040047, 0x00000008, 0x0000001e, 0x00000000,
    // %void = OpTypeVoid
    0x00020013, 0x00000002,
    // %voidfn = OpTypeFunction %void
    0x00030021, 0x00000003, 0x00000002,
    // %float = OpTypeFloat 32
    0x00030016, 0x00000005, 0x00000020,
    // %v4float = OpTypeVector %float 4
    0x00040017, 0x00000006, 0x00000005, 0x00000004,
    // %ptr_Output_v4float = OpTypePointer Output %v4float
    0x00040020, 0x00000007, 0x00000003, 0x00000006,
    // %outColor = OpVariable Output
    0x00040003b, 0x00000007, 0x00000008, 0x00000003,
    // constants: 0.6, 0.7, 0.8, 1.0
    0x0004002b, 0x00000005, 0x00000009, 0x3f19999a, // 0.6f
    0x0004002b, 0x00000005, 0x0000000a, 0x3f333333, // 0.7f
    0x0004002b, 0x00000005, 0x0000000b, 0x3f4ccccd, // 0.8f
    0x0004002b, 0x00000005, 0x0000000c, 0x3f800000, // 1.0f
    // %color = OpConstantComposite %v4float 0.6 0.7 0.8 1.0
    0x00070032, 0x00000006, 0x0000000d,
               0x00000009, 0x0000000a, 0x0000000b, 0x0000000c,
    // %main = OpFunction %void None %voidfn
    0x00050036, 0x00000002, 0x00000001, 0x00000000, 0x00000003,
    // %entry = OpLabel
    0x000200f8, 0x0000000e,
    // OpStore %outColor %color
    0x0003003e, 0x00000008, 0x0000000d,
    // OpReturn
    0x000100fd,
    // OpFunctionEnd
    0x00010038
};
// clang-format on

static constexpr uint32_t k_geometryVertSPIRVSize =
    static_cast<uint32_t>(sizeof(k_geometryVertSPIRV) / sizeof(uint32_t));

static constexpr uint32_t k_geometryFragSPIRVSize =
    static_cast<uint32_t>(sizeof(k_geometryFragSPIRV) / sizeof(uint32_t));
