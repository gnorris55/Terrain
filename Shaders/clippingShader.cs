#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D clippingTex;

//uniform vec3 position;
uniform mat4 projView;
uniform mat4 model;

void main()
{
    vec3 position;
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    position.x = texelCoord.x*2;
    position.y = 0;
    position.z = texelCoord.y*2;
    
    vec4 point_c = projView * vec4(position, 1.0)*model;
    value.x = point_c.x / point_c.w;
    value.y = point_c.y / point_c.w;
    value.z = point_c.z / point_c.w;
    
    //imageStore(clippingTex, texelCoord, vec4(position, 1.0)*model);
    //imageStore(clippingTex, texelCoord, point_c);
    //imageStore(clippingTex, texelCoord, value);

}