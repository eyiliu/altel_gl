#version 150 core

layout(points) in;
in vec3 vColor[];

layout(line_strip, max_vertices = 2) out;
out vec3 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

layout (std140) uniform UniformLayer{
  vec3  position; //
  vec3  color;    //
  vec3  pitch;    // pitch_x pitch_y pitch_z/thick_z
  uvec3 npixel;   // pixel_x pixel_y pixel_z/always1
  mat4  trans;    // 
} layers[10];

void main(){
  int    nlx = int(gl_in[0].gl_Position.z+0.1);
  vec3   pos;
  vec3   color;
  vec3   pitch;
  uvec3  npixel;
  mat4   trans;
  switch(nlx){
  case 1:
    pos    = layers[0].position;
    color  = layers[0].color;
    pitch  = layers[0].pitch;
    npixel = layers[0].npixel;
    trans  = layers[0].trans;
    break;
  case 0:
    pos    = layers[1].position;
    color  = layers[1].color;
    pitch  = layers[1].pitch;
    npixel = layers[1].npixel;
    trans  = layers[1].trans;
    break;
  case 2:
    pos    = layers[2].position;
    color  = layers[2].color;
    pitch  = layers[2].pitch;
    npixel = layers[2].npixel;
    trans  = layers[2].trans;
    break;
  case 3:
    pos    = layers[3].position;
    color  = layers[3].color;
    pitch  = layers[3].pitch;
    npixel = layers[3].npixel;
    trans  = layers[3].trans;
    break;
  case 4:
    pos    = layers[4].position;
    color  = layers[4].color;
    pitch  = layers[4].pitch;
    npixel = layers[4].npixel;
    trans  = layers[4].trans;
    break;
  case 5:
    pos    = layers[5].position;
    color  = layers[5].color;
    pitch  = layers[5].pitch;
    npixel = layers[5].npixel;
    trans  = layers[5].trans;
    break;
  case 6:
    pos    = layers[6].position;
    color  = layers[6].color;
    pitch  = layers[6].pitch;
    npixel = layers[6].npixel;
    trans  = layers[6].trans;
    break;
  case 7:
    pos    = layers[7].position;
    color  = layers[7].color;
    pitch  = layers[7].pitch;
    npixel = layers[7].npixel;
    trans  = layers[7].trans;
    break;
  case 8:
    pos    = layers[8].position;
    color  = layers[8].color;
    pitch  = layers[8].pitch;
    npixel = layers[8].npixel;
    trans  = layers[8].trans;
    break;
  case 9:
    pos    = layers[9].position;
    color  = layers[9].color;
    pitch  = layers[9].pitch;
    npixel = layers[9].npixel;
    trans  = layers[9].trans;
    break;
  default:
    pos    = layers[5].position;
    color  = layers[5].color;
    pitch  = layers[5].pitch;
    npixel = layers[5].npixel;
    trans  = layers[5].trans;
    break;
  }


  vec3 pos_hit = pos.xyz + vec3(pitch.xy * gl_in[0].gl_Position.xy , 0); 
  //fColor = vColor[0];
  fColor = color;

  mat4 pvmMatrix = proj * view * model * trans;
  vec4 gp = pvmMatrix * vec4(pos_hit, 1.0);
  vec4 offset = pvmMatrix * vec4( 0.0,  0.0,  1.0, 0.0);

  gl_Position = gp + offset;
  EmitVertex();

  gl_Position = gp - offset;
  EmitVertex();

  EndPrimitive();
}
