#version 430 core

layout(points) in;
in vec3 vColor[];

layout(line_strip, max_vertices = 2) out;
out vec3 fColor;

layout (std140) struct GeoDataLayer{
  ivec4 id; // id, rev , rev, rev
  vec4  pos; // vec3, pad
  vec4  size;     // size_x, size_y, size_z, pad
  vec4  color;    // vec3, pad
  vec4  pitch;    // pitch_x pitch_y pitch_z/thick_z
  uvec4 npixel;   // pixel_x pixel_y pixel_z/always1
  mat4  trans;    // model
};

layout (std140) uniform GeoData{
  GeoDataLayer  ly[20]; //
} geoData;

layout (std140) uniform TransformData{
  mat4 model;
  mat4 view;
  mat4 proj;
} transformData;

void main(){

  vec3 pos_hit = gl_in[0].gl_Position.xyz;
  mat4 pvmMatrix = transformData.proj * transformData.view * transformData.model;

  vec4 gp = pvmMatrix * vec4(pos_hit, 1.0);
  vec4 offset = pvmMatrix * vec4( 0.0,  0.0,  8.0, 0.0);

  fColor = vColor[0].rgb;

  gl_Position = gp + offset;
  EmitVertex();

  gl_Position = gp - offset;
  EmitVertex();

  EndPrimitive();
}
