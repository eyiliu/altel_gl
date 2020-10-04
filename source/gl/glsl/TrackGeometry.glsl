#version 150 core

layout(points) in;
in vec3 vColor[];

layout(line_strip, max_vertices = 2) out;
out vec3 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(){
  int    nlx = int(gl_in[0].gl_Position.z+0.1);
  vec3   pos;
  mat4   trans;

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
