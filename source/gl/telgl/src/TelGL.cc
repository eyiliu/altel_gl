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
  if(!vertex_glsl.empty()){
    m_shader_vertex = createShader(GL_VERTEX_SHADER, vertex_glsl.c_str());
  }
  else{
    m_shader_vertex = createShader(GL_VERTEX_SHADER, default_vertex_glsl);
  }

  if(!geometry_glsl.empty()){
    m_shader_geometry = createShader(GL_GEOMETRY_SHADER, geometry_glsl.c_str());
  }
  else{
    m_shader_geometry = createShader(GL_GEOMETRY_SHADER, default_geometry_glsl);
  }
  if(!fragment_glsl.empty()){
    m_shader_fragment = createShader(GL_FRAGMENT_SHADER, fragment_glsl.c_str());
  }
  else{
    m_shader_fragment = createShader(GL_FRAGMENT_SHADER, default_fragment_glsl);
  }

  //// create buffer object
  glGenBuffers(1, &m_vbuffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbuffer_id);
  // assign storage to buffer object
  glNamedBufferData(m_vbuffer_id, sizeof(m_data_id), NULL, GL_STATIC_DRAW);

  //// create buffer object
  glGenBuffers(1, &m_ubuffer_geodata);
  glBindBuffer(GL_UNIFORM_BUFFER, m_ubuffer_geodata);
  // assign storage to buffer object
  glNamedBufferData(m_ubuffer_geodata, sizeof(m_data_geodata), NULL, GL_STATIC_DRAW);
  // assign a bind point to buffer object (and specify the data storage range)
  m_bindpoint_geodata = 0;
  glBindBufferRange(GL_UNIFORM_BUFFER, m_bindpoint_geodata, m_ubuffer_geodata, 0, sizeof(m_data_geodata));

  /////// tel program
  m_program = glCreateProgram();
  glAttachShader(m_program, m_shader_vertex);
  glAttachShader(m_program, m_shader_geometry);
  glAttachShader(m_program, m_shader_fragment);
  glLinkProgram(m_program);

  //// create vertex array object and connect shader program variable to it
  glGenVertexArrays(1, &m_vertex_array_object);
  glBindVertexArray(m_vertex_array_object);
  m_location_id = glGetAttribLocation(m_program, "l");
  glEnableVertexAttribArray(m_location_id);
  glVertexAttribIPointer(m_location_id, 1, GL_INT, sizeof(GLint), 0);

  //// connect shader progream varible to a bind point of an ubuffer
  m_blockindex_geodata = glGetUniformBlockIndex(m_program, "GeoData");
  glUniformBlockBinding(m_program, m_blockindex_geodata, m_bindpoint_geodata);

  m_location_model = glGetUniformLocation(m_program, "model");
  m_location_view = glGetUniformLocation(m_program, "view");
  m_location_proj = glGetUniformLocation(m_program, "proj");

  updateGeoData(geo_js_string);
}

void TelGL::lookAt(float cameraX, float cameraY, float cameraZ,
                   float centerX, float centerY, float centerZ,
                   float upvectX, float upvectY, float upvectZ,
                   float fovHoriz, float nearDist, float farDist,
                   float ratioWidthHigh
                   ){
  glUseProgram(m_program);
  m_data_model = glm::mat4( 1.0f );
  m_data_view = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ),    // eye position
                            glm::vec3(centerX, centerY, centerZ),    // object center
                            glm::vec3(upvectX, upvectY, upvectZ));   // up vector
  m_data_proj = glm::perspective(glm::radians(fovHoriz), ratioWidthHigh, nearDist, farDist);

  glUniformMatrix4fv(m_location_model, 1, GL_FALSE, glm::value_ptr(m_data_model));
  glUniformMatrix4fv(m_location_view, 1, GL_FALSE, glm::value_ptr(m_data_view));
  glUniformMatrix4fv(m_location_proj, 1, GL_FALSE, glm::value_ptr(m_data_proj));
}

TelGL::~TelGL(){
  if(m_program) {glDeleteProgram(m_program); m_program = 0;}
  if(m_shader_fragment){glDeleteShader(m_shader_fragment); m_shader_fragment = 0;}
  if(m_shader_geometry){glDeleteShader(m_shader_geometry); m_shader_geometry = 0;}
  if(m_shader_vertex){glDeleteShader(m_shader_vertex); m_shader_vertex = 0;}
  if(m_vertex_array_object){glDeleteVertexArrays(1, &m_vertex_array_object); m_vertex_array_object = 0;}
  if(m_vbuffer_id){glDeleteBuffers(1, &m_vbuffer_id); m_vbuffer_id = 0;}
  // for(int i=0; i<m_points.size(); i++)
  //   if(m_uboLayers[i]){glDeleteBuffers(1, &m_uboLayers[i]); m_uboLayers[i] = 0;}
}

void TelGL::updateGeoData(const std::string & geo_js_string){
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
    m_data_id[n] = id; //
    m_data_geodata[n].id[0] = id;
    m_data_geodata[n].pos[0]=  cx;
    m_data_geodata[n].pos[1]=  cy;
    m_data_geodata[n].pos[2]=  cz;
    m_data_geodata[n].pos[3]=  0;
    m_data_geodata[n].size[0] = hx*2;
    m_data_geodata[n].size[1] = hy*2;
    m_data_geodata[n].size[2] = hz*2;
    m_data_geodata[n].size[3] = 0;
    m_data_geodata[n].color[0]= colorR;
    m_data_geodata[n].color[1]= colorG;
    m_data_geodata[n].color[2]= colorB;
    m_data_geodata[n].color[3]= 0;
    m_data_geodata[n].pitch[0]= hx*2/pxn;
    m_data_geodata[n].pitch[1]= hy*2/pyn;
    m_data_geodata[n].pitch[2]= hz*2/1;
    m_data_geodata[n].pitch[3]= 0;
    m_data_geodata[n].npixel[0]= pxn;
    m_data_geodata[n].npixel[1]= pyn;
    m_data_geodata[n].npixel[2]= 1;
    m_data_geodata[n].npixel[3]= 0;
    n++;
    if(n>=sizeof(m_data_geodata)/sizeof(TelGL::GeoDataGL)){
      std::fprintf(stdout, "TelGL:: Max size of geodata is reached, ignoring later geodata !\n");
      break;
    }
  }
  m_counter_geodata = n;
  // glBindBufferRange(GL_UNIFORM_BUFFER, bind_point, m_ubuffer_geodata, 0, sizeof(m_data_geodata));
  // glUseProgram(m_program);
  glNamedBufferSubData(m_ubuffer_geodata, 0, sizeof(TelGL::GeoDataGL)*m_counter_geodata, &m_data_geodata[0]);
  std::fprintf(stdout, "set up geometry data with %i layers\n", m_counter_geodata);
}

void TelGL::draw(int layer){
  GLint id = layer;
  glUseProgram(m_program);
  glBindVertexArray(m_vertex_array_object);
  glNamedBufferSubData(m_vbuffer_id, 0, sizeof(GLint)*1, &id);
  glDrawArrays(GL_POINTS, 0, 1);
}

void TelGL::draw(){
  glUseProgram(m_program);
  glBindVertexArray(m_vertex_array_object);
  glNamedBufferSubData(m_vbuffer_id, 0, sizeof(GLint)*m_counter_geodata, m_data_id);
  glDrawArrays(GL_POINTS, 0, m_counter_geodata);
}
