#version 430 core

layout(points) in;
in vec3 vColor[];

layout(line_strip, max_vertices = 2) out;
out vec3 fColor;

layout (std140) struct GeoDataLayer{
  ivec4 id; // id, rev , rev, rev
  vec4  position; // vec3, pad
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
  mat4 modelx;
  mat4 viewx;
  mat4 projx;
} transformData;


void main(){
  int    nlx = int(gl_in[0].gl_Position.z+0.1);
  vec3   pos;
  vec3   thick;
  vec3   color;
  vec3   pitch;
  uvec3  npixel;
  mat4   trans;

  int  n;
  n = -1;
  for (int k = 0; k < 20; ++k){
    if(nlx == geoData.ly[k].id.x ){
      n = k;
      break;
    }
  }
  if(n == -1){
    return ;
  }

  for (int k = 0; k < 20; ++k){
    if(n ==k){
      pos    = geoData.ly[k].position.xyz;
      thick  = geoData.ly[k].size.xyz;
      color  = geoData.ly[k].color.xyz;
      pitch  = geoData.ly[k].pitch.xyz;
      npixel = geoData.ly[k].npixel.xyz;
      trans  = geoData.ly[k].trans;
      break;
    }
  }

  vec3 pos_hit = pos.xyz - (pitch.xyz*npixel.xyz /2) + vec3( pitch.xy * gl_in[0].gl_Position.xy, 0);
  //fColor = vColor[0];
  fColor = color;

  mat4 pvmMatrix = transformData.projx * transformData.viewx * transformData.modelx * trans;

  vec4 gp = pvmMatrix * vec4(pos_hit, 1.0);
  vec4 offset = pvmMatrix * vec4( 0.0,  0.0,  8.0, 0.0);

  gl_Position = gp + offset;
  EmitVertex();

  gl_Position = gp - offset;
  EmitVertex();

  EndPrimitive();
}
