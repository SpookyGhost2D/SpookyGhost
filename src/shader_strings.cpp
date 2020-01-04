#include "shader_strings.h"

char const *const ShaderStrings::texture_vs =
    "uniform vec2 size;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 modelView;\n"
    "out vec2 vTexCoords;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	vec2 aPosition = vec2(0.5 - float(gl_VertexID >> 1), -0.5 + float(gl_VertexID % 2));\n"
    "	vec2 aTexCoords = vec2(1.0 - float(gl_VertexID >> 1), float(gl_VertexID % 2));\n"
    "	vec4 position = vec4(aPosition.x * size.x, aPosition.y * size.y, 0.0, 1.0);\n"
    "\n"
    "	gl_Position = projection * modelView * position;\n"
    "	vTexCoords = vec2(aTexCoords.x, aTexCoords.y);\n"
    "}\n";

char const *const ShaderStrings::texture_fs =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "uniform sampler2D uTexture;\n"
    "in vec2 vTexCoords;\n"
    "out vec4 fragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	fragColor = texture(uTexture, vTexCoords);\n"
    "}\n";

char const *const ShaderStrings::sprite_vs =
    "uniform vec4 texRect;\n"
    "uniform vec2 spriteSize;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 modelView;\n"
    "out vec2 vTexCoords;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	vec2 aPosition = vec2(0.5 - float(gl_VertexID >> 1), -0.5 + float(gl_VertexID % 2));\n"
    "	vec2 aTexCoords = vec2(1.0 - float(gl_VertexID >> 1), float(gl_VertexID % 2));\n"
    "	vec4 position = vec4(aPosition.x * spriteSize.x, aPosition.y * spriteSize.y, 0.0, 1.0);\n"
    "\n"
    "	gl_Position = projection * modelView * position;\n"
    "	vTexCoords = vec2(aTexCoords.x * texRect.x + texRect.y, aTexCoords.y * texRect.z + texRect.w);\n"
    "}\n";

char const *const ShaderStrings::sprite_fs =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "uniform sampler2D uTexture;\n"
    "in vec2 vTexCoords;\n"
    "out vec4 fragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	fragColor = texture(uTexture, vTexCoords);\n"
    "}\n";

char const *const ShaderStrings::meshsprite_vs =
    "uniform vec4 texRect;\n"
    "uniform vec2 spriteSize;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 modelView;\n"
    "in vec2 aPosition;\n"
    "in vec2 aTexCoords;\n"
    "out vec2 vTexCoords;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	vec4 position = vec4(aPosition.x * spriteSize.x, aPosition.y * spriteSize.y, 0.0, 1.0);\n"
    "	gl_Position = projection * modelView * position;\n"
    "	vTexCoords = vec2(aTexCoords.x * texRect.x + texRect.y, aTexCoords.y * texRect.z + texRect.w);\n"
    "}\n";

#if 1
char const *const ShaderStrings::meshsprite_snap_vs =
    "uniform vec4 texRect;\n"
    "uniform vec2 spriteSize;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 modelView;\n"
    "in vec2 aPosition;\n"
    "in vec2 aTexCoords;\n"
    "out vec2 vTexCoords;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	vec4 position = vec4(aPosition.x * spriteSize.x, aPosition.y * spriteSize.y, 0.0, 1.0);\n"
    "	gl_Position = modelView * position;\n"
    "\n"
    "	gl_Position.x = round(gl_Position.x);\n"
    "\n"
    "	gl_Position.y = round(gl_Position.y);\n"
    "	gl_Position = projection * gl_Position;\n"
    "\n"
    "	vTexCoords = vec2(aTexCoords.x * texRect.x + texRect.y, aTexCoords.y * texRect.z + texRect.w);\n"
    "}\n";
#endif
