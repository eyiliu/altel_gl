#version 430 core

layout (std140) uniform IdArray{
  vec4  id[8];    //v0=id, v1=n
} idArray;

in  int  l;
out int  nl;

void main(){
  nl = l;
}
