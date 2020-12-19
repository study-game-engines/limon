#version 330 core

layout (location = 0) out vec4 color;

layout (std140) uniform PlayerTransformBlock {
    mat4 camera;
    mat4 projection;
    mat4 cameraProjection;
    mat4 inverseProjection;
    mat4 inverseCamera;
    vec3 position;
    vec3 cameraSpacePosition;
    vec2 noiseScale;
} playerTransforms;

in VS_FS {
    vec2 textureCoordinates;
} from_vs;

uniform sampler2D sprite;
uniform sampler2D pre_depthMap;


void main(void){
    vec4 textureColor = (texture(sprite, from_vs.textureCoordinates));

    textureColor.xyz = textureColor.xyz * textureColor.a;
    color = textureColor;

}