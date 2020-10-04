#version 150 core

in ivec3 pos;

out vec3 vColor;

void main()
{
  gl_Position = vec4(float(pos.x), float(pos.y), float(pos.z), 1.0);
  vColor = vec3(0, 1, 0);
}

