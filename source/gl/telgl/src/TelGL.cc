#include "myrapidjson.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <sstream>

#include "TelGL.hh"
#include "glm/ext.hpp"

namespace {
  const GLchar* default_vertex_glsl =
#include "TelVertex_glsl.hh"
  ;
// // Geometry shader
  const GLchar* default_geometry_glsl =
#include "TelGeometry_glsl.hh"
  ;
// Fragment shader
  const GLchar* default_fragment_glsl =
#include "TelFragment_glsl.hh"
  ;

  GLuint createShader(GLenum type, const GLchar* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint IsCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &IsCompiled);
    if(IsCompiled == GL_FALSE){
      GLint maxLength;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
      std::vector<GLchar> infoLog(maxLength, 0);
      glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());
      std::fprintf(stderr, "ERROR is catched at compile time of opengl GLSL.\n%s", infoLog.data());
      std::fprintf(stderr, "======problematic GLSL code below=====\n%s\n=====end of GLSL code====\n", src);
      throw;
    }
    return shader;
  }
}

TelGL::TelGL(const std::string& geo_js_string,
             const std::string& vertex_glsl,
             const std::string& geometry_glsl,
             const std::string& fragment_glsl
             ){

  //// create buffer object  (geometry)
  glGenBuffers(1, &m_ubuffer_geo);
  // assign buffer as a vbuffer
  glBindBuffer(GL_UNIFORM_BUFFER, m_ubuffer_geo);
  // assign storage to buffer object
  glNamedBufferData(m_ubuffer_geo, sizeof(m_data_geo), NULL, GL_STATIC_DRAW);
  // assign a bind point to buffer object (and specify the data storage range)
  m_bindpoint_geo = 0;
  glBindBufferRange(GL_UNIFORM_BUFFER, m_bindpoint_geo, m_ubuffer_geo, 0, sizeof(m_data_geo));

  //// create buffer object  (transform)
  glGenBuffers(1, &m_ubuffer_transform);
  // assign buffer as a vbuffer
  glBindBuffer(GL_UNIFORM_BUFFER, m_ubuffer_transform);
  // assign storage to buffer object
  glNamedBufferData(m_ubuffer_transform, sizeof(m_data_transform), NULL, GL_STATIC_DRAW);
  // assign a bind point to buffer object (and specify the data storage range)
  m_bindpoint_transform = 1;
  glBindBufferRange(GL_UNIFORM_BUFFER, m_bindpoint_transform, m_ubuffer_transform, 0, sizeof(m_data_transform));

  //////////////////////////////telescope///////////////////////////////////////
  ////// telescope data
  //// create vertex array object
  glGenVertexArrays(1, &m_vertex_array_tel);
  //// create buffer object
  glGenBuffers(1, &m_vbuffer_tel_id);
  //// assign buffer as a vbuffer in a vertex array object
  glBindVertexArray(m_vertex_array_tel);   // bind vao to GL_ARRAY_BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, m_vbuffer_tel_id);// bind vbo to GL_ARRAY_BUFFER
  //assign storage to buffer object
  glNamedBufferData(m_vbuffer_tel_id, sizeof(m_data_tel_id), NULL, GL_STATIC_DRAW);

  ////// telescope program
  //// shader and variables
  // create program
  m_shader_vertex_tel   = createShader(GL_VERTEX_SHADER,   (vertex_glsl.empty()?   default_vertex_glsl  :vertex_glsl.c_str()));
  m_shader_geometry_tel = createShader(GL_GEOMETRY_SHADER, (geometry_glsl.empty()? default_geometry_glsl:geometry_glsl.c_str()));
  m_shader_fragment_tel = createShader(GL_FRAGMENT_SHADER, (fragment_glsl.empty()? default_fragment_glsl:fragment_glsl.c_str()));
  m_program_tel = glCreateProgram();
  glAttachShader(m_program_tel, m_shader_vertex_tel);
  glAttachShader(m_program_tel, m_shader_geometry_tel);
  glAttachShader(m_program_tel, m_shader_fragment_tel);
  glLinkProgram(m_program_tel);
  // get attributes from program
  m_blockindex_geo_tel = glGetUniformBlockIndex(m_program_tel, "GeoData");
  m_blockindex_transform_tel = glGetUniformBlockIndex(m_program_tel, "TransformData");
  m_location_tel_id = glGetAttribLocation(m_program_tel, "l");
  glVertexAttribIPointer(m_location_tel_id, 1, GL_INT, sizeof(GLint), 0);
  // connect attributes to buffer
  //// connect uniform block to bind point (ubuffer)
  glUniformBlockBinding(m_program_tel, m_blockindex_geo_tel, m_bindpoint_geo);
  glUniformBlockBinding(m_program_tel, m_blockindex_transform_tel, m_bindpoint_transform);
  //// connect vertex attribute to vertex array object (vbuffer)
  glBindVertexArray(m_vertex_array_tel);
  glEnableVertexAttribArray(m_location_tel_id); //use currently bound vertex array object for the operation
  //////////////////////////////end of telescope////////////////////////////////////

  updateGeometry(geo_js_string);
}

TelGL::~TelGL(){
  ////// telescope
  if(m_program_tel) {glDeleteProgram(m_program_tel); m_program_tel = 0;}
  if(m_shader_fragment_tel){glDeleteShader(m_shader_fragment_tel); m_shader_fragment_tel = 0;}
  if(m_shader_geometry_tel){glDeleteShader(m_shader_geometry_tel); m_shader_geometry_tel = 0;}
  if(m_shader_vertex_tel){glDeleteShader(m_shader_vertex_tel); m_shader_vertex_tel = 0;}
  if(m_vertex_array_tel){glDeleteVertexArrays(1, &m_vertex_array_tel); m_vertex_array_tel = 0;}
  if(m_vbuffer_tel_id){glDeleteBuffers(1, &m_vbuffer_tel_id); m_vbuffer_tel_id = 0;}
  //
  // ubuffer

}


void TelGL::updateTransform(float cameraX, float cameraY, float cameraZ,
                            float centerX, float centerY, float centerZ,
                            float upvectX, float upvectY, float upvectZ,
                            float fovHoriz, float nearDist, float farDist,
                            float ratioWidthHigh){
  m_data_transform.model = glm::mat4( 1.0f );
  m_data_transform.view = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ),    // eye position
                                      glm::vec3(centerX, centerY, centerZ),    // object center
                                      glm::vec3(upvectX, upvectY, upvectZ));   // up vector
  m_data_transform.proj = glm::perspective(glm::radians(fovHoriz), ratioWidthHigh, nearDist, farDist);

  glNamedBufferSubData(m_ubuffer_transform, 0, sizeof(m_data_transform), &m_data_transform);
}

void TelGL::updateGeometry(const std::string & geo_js_string){
  std::istringstream iss(geo_js_string);
  rapidjson::IStreamWrapper isw(iss);
  rapidjson::Document doc_geo;
  doc_geo.ParseStream(isw);
  if(!doc_geo.HasMember("geo")){
    std::fprintf(stderr, "geo json file error\n");
    exit(1);
  }

  const auto &js_geo = doc_geo["geo"];
  size_t n = 0;
  for(const auto& l: js_geo.GetArray()){
    size_t id = l["id"].GetUint();
    double cx = l["centerX"].GetDouble();
    double cy = l["centerY"].GetDouble();
    double cz = l["centerZ"].GetDouble();
    double rx = l["rotX"].GetDouble();
    double ry = l["rotY"].GetDouble();
    double rz = l["rotZ"].GetDouble();
    double pxn = l["pixelXN"].GetDouble();
    double pyn = l["pixelYN"].GetDouble();
    double hx = l["halfX"].GetDouble();
    double hy = l["halfY"].GetDouble();
    double hz = l["halfZ"].GetDouble();

    double colorR = 0;
    double colorG = 0;
    double colorB = 1;

    if(l.HasMember("colorRGB") &&
       l["colorRGB"].IsArray() &&
       l["colorRGB"].Size() == 3){
      colorR = l["colorRGB"][0].GetDouble();
      colorG = l["colorRGB"][1].GetDouble();
      colorB = l["colorRGB"][2].GetDouble();
    }
    m_data_tel_id[n] = id; //
    m_data_geo[n].id[0] = id;
    m_data_geo[n].pos[0]=  cx;
    m_data_geo[n].pos[1]=  cy;
    m_data_geo[n].pos[2]=  cz;
    m_data_geo[n].pos[3]=  0;
    m_data_geo[n].size[0] = hx*2;
    m_data_geo[n].size[1] = hy*2;
    m_data_geo[n].size[2] = hz*2;
    m_data_geo[n].size[3] = 0;
    m_data_geo[n].color[0]= colorR;
    m_data_geo[n].color[1]= colorG;
    m_data_geo[n].color[2]= colorB;
    m_data_geo[n].color[3]= 0;
    m_data_geo[n].pitch[0]= hx*2/pxn;
    m_data_geo[n].pitch[1]= hy*2/pyn;
    m_data_geo[n].pitch[2]= hz*2/1;
    m_data_geo[n].pitch[3]= 0;
    m_data_geo[n].npixel[0]= pxn;
    m_data_geo[n].npixel[1]= pyn;
    m_data_geo[n].npixel[2]= 1;
    m_data_geo[n].npixel[3]= 0;
    n++;
    if(n>=sizeof(m_data_geo)/sizeof(TelGL::GeoDataGL)){
      std::fprintf(stdout, "TelGL:: Max size of geo is reached, ignoring later geo !\n");
      break;
    }
  }
  m_counter_geo = n;
  // glBindBufferRange(GL_UNIFORM_BUFFER, bind_point, m_ubuffer_geo, 0, sizeof(m_data_geo));
  // glUseProgram(m_program);
  glNamedBufferSubData(m_ubuffer_geo, 0, sizeof(TelGL::GeoDataGL)*m_counter_geo, &m_data_geo[0]);
  std::fprintf(stdout, "set up geometry data with %i layers\n", m_counter_geo);
}

void TelGL::draw(int layer){
  GLint id = layer;
  glUseProgram(m_program_tel);
  glBindVertexArray(m_vertex_array_tel);
  glNamedBufferSubData(m_vbuffer_tel_id, 0, sizeof(GLint)*1, &id);
  glDrawArrays(GL_POINTS, 0, 1);
}

void TelGL::draw(){
  glUseProgram(m_program_tel);
  glBindVertexArray(m_vertex_array_tel);
  glNamedBufferSubData(m_vbuffer_tel_id, 0, sizeof(GLint)*m_counter_geo, m_data_tel_id);
  glDrawArrays(GL_POINTS, 0, m_counter_geo);
}
