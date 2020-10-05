#version 430 core

layout(points) in;
layout(line_strip, max_vertices = 16) out;

layout (std140) struct GeoDataLayer{
  vec4  position; //
  vec4  color;    //
  vec4  pitch;    // pitch_x pitch_y pitch_z/thick_z
  uvec4 npixel;   // pixel_x pixel_y pixel_z/always1
  mat4  trans;    //
};

layout (std140) uniform GeoData{
  GeoDataLayer  ly[8]; //
} geoData;

in int nl[];
out vec3 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(){
  vec3   pos;
  vec3   color;
  vec3   pitch;
  uvec3  npixel;
  mat4   trans;
  int   id;
  
  // for (int k = 0; k < 100; ++k){
  //   if(nl[0]==k){
  //     id = k;
  //   }
  // }

  for (int k = 0; k < 8; ++k){
    if(nl[0]==k){
      pos    = geoData.ly[k].position.xyz;
      color  = geoData.ly[k].color.xyz;
      pitch  = geoData.ly[k].pitch.xyz;
      npixel = geoData.ly[k].npixel.xyz;
      trans  = geoData.ly[k].trans;
    }
  }
  fColor = color;

  mat4 pvmMatrix = proj * view * model * trans;
  vec4 gp = pvmMatrix * vec4(pos, 1.0);
  vec3 thick  = pitch.xyz * npixel.xyz;
  //                                     N/S       U/D       E/W
  //                                     X         Y         Z
  vec4 NEU = pvmMatrix * vec4( thick.x,  thick.y,  thick.z, 0.0);
  vec4 NED = pvmMatrix * vec4( thick.x,  0,        thick.z, 0.0);
  vec4 NWU = pvmMatrix * vec4( thick.x,  thick.y,  0,       0.0);
  vec4 NWD = pvmMatrix * vec4( thick.x,  0,        0,       0.0);
  vec4 SEU = pvmMatrix * vec4( 0,        thick.y,  thick.z, 0.0);
  vec4 SED = pvmMatrix * vec4( 0,        0,        thick.z, 0.0);
  vec4 SWU = pvmMatrix * vec4( 0,        thick.y,  0,       0.0);
  vec4 SWD = pvmMatrix * vec4( 0,        0,        0,       0.0);
  gl_Position = gp + NED;
  EmitVertex();
  gl_Position = gp + NWD;
  EmitVertex();
  gl_Position = gp + SWD;
  EmitVertex();
  gl_Position = gp + SED;
  EmitVertex();
  gl_Position = gp + SEU;
  EmitVertex();
  gl_Position = gp + SWU;
  EmitVertex();
  gl_Position = gp + NWU;
  EmitVertex();
  gl_Position = gp + NEU;
  EmitVertex();
  gl_Position = gp + NED;
  EmitVertex();
  gl_Position = gp + SED;
  EmitVertex();
  gl_Position = gp + SEU;
  EmitVertex();
  gl_Position = gp + NEU;
  EmitVertex();
  gl_Position = gp + NWU;
  EmitVertex();
  gl_Position = gp + NWD;
  EmitVertex();
  gl_Position = gp + SWD;
  EmitVertex();
  gl_Position = gp + SWU;
  EmitVertex();
  EndPrimitive();
}
