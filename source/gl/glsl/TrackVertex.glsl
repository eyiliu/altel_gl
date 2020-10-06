#version 430 core

in ivec3 pos;

out vec3 vColor;

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
  GeoDataLayer geo;
  bool found = false;
  for (int k = 0; k < 20; ++k){
    if(pos.z == geoData.ly[k].id.x ){
      geo = geoData.ly[k];
      found = true;
      break;
    }
  }
  if(!found){
    return ;
  }
  vec4 pos_global = geo.trans * vec4(geo.pos.xyz - (geo.pitch.xyz*geo.npixel.xyz /2) + vec3( geo.pitch.xy * pos.xy, 0),  1.0);

  mat4 pvmMatrix = transformData.proj * transformData.view * transformData.model;

  gl_Position = pvmMatrix * vec4(pos_global.xyz, 1.0)
  vColor = geo.color.rgb;
}
