
#include "myrapidjson.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <sstream>

#include "TelGL.hh"
#include "glm/ext.hpp"

namespace {
  // default tel
  const GLchar* default_vertex_glsl =
#include "TelVertex_glsl.hh"
  ;
  const GLchar* default_geometry_glsl =
#include "TelGeometry_glsl.hh"
  ;
  const GLchar* default_fragment_glsl =
#include "TelFragment_glsl.hh"
  ;

  // default hit
  const GLchar* default_vertex_glsl_hit =
#include "HitVertex_glsl.hh"
  ;
  const GLchar* default_geometry_glsl_hit =
#include "HitGeometry_glsl.hh"
  ;
  const GLchar* default_fragment_glsl_hit =
#include "TelFragment_glsl.hh" // using Tel
  ;

  // default track
  const GLchar* default_vertex_glsl_track =
#include "TrackVertex_glsl.hh"
  ;
  const GLchar* default_fragment_glsl_track =
#include "TelFragment_glsl.hh" // using Tel
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

TelGL::TelGL(const JsonValue &js){
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
  m_shader_vertex_tel   = createShader(GL_VERTEX_SHADER,  default_vertex_glsl);
  m_shader_geometry_tel = createShader(GL_GEOMETRY_SHADER, default_geometry_glsl);
  m_shader_fragment_tel = createShader(GL_FRAGMENT_SHADER, default_fragment_glsl);
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

  //////////////////////////////hit///////////////////////////////////////
  ////// hit data
  //// create vertex array object
  glGenVertexArrays(1, &m_vertex_array_hit);
  //// create buffer object
  glGenBuffers(1, &m_vbuffer_hit_pos);
  //// assign buffer as a vbuffer in a vertex array object
  glBindVertexArray(m_vertex_array_hit);   // bind vao to GL_ARRAY_BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, m_vbuffer_hit_pos);// bind vbo to GL_ARRAY_BUFFER
  //assign storage to buffer object, MAX 1024 hits
  glNamedBufferData(m_vbuffer_hit_pos, sizeof(GLfloat)*4*1024, NULL, GL_STATIC_DRAW);
  ////// program
  //// shader and variables
  // create program
  m_shader_vertex_hit   = createShader(GL_VERTEX_SHADER,   default_vertex_glsl_hit);
  m_shader_geometry_hit = createShader(GL_GEOMETRY_SHADER, default_geometry_glsl_hit);
  m_shader_fragment_hit = createShader(GL_FRAGMENT_SHADER, default_fragment_glsl_hit);
  m_program_hit = glCreateProgram();
  glAttachShader(m_program_hit, m_shader_vertex_hit);
  glAttachShader(m_program_hit, m_shader_geometry_hit);
  glAttachShader(m_program_hit, m_shader_fragment_hit);
  glLinkProgram(m_program_hit);
  // get attributes from program
  m_blockindex_geo_hit = glGetUniformBlockIndex(m_program_hit, "GeoData");
  m_blockindex_transform_hit = glGetUniformBlockIndex(m_program_hit, "TransformData");
  m_location_hit_pos = glGetAttribLocation(m_program_hit, "pos");
  glVertexAttribPointer(m_location_hit_pos, 4, GL_FLOAT, GL_FALSE, sizeof(GLint)*4, 0);
  // connect attributes to buffer
  //// connect uniform block to bind point (ubuffer)
  glUniformBlockBinding(m_program_hit, m_blockindex_geo_hit, m_bindpoint_geo);
  glUniformBlockBinding(m_program_hit, m_blockindex_transform_hit, m_bindpoint_transform);
  //// connect vertex attribute to vertex array object (vbuffer)
  glBindVertexArray(m_vertex_array_hit);
  glEnableVertexAttribArray(m_location_hit_pos); //use currently bound vertex array object for the operation
  //////////////////////////////end of hit////////////////////////////////////


  //////////////////////////////track///////////////////////////////////////
  ////// data
  //// create vertex array object
  glGenVertexArrays(1, &m_vertex_array_track);
  //// create buffer object
  glGenBuffers(1, &m_vbuffer_track_pos);
  //// assign buffer as a vbuffer in a vertex array object
  glBindVertexArray(m_vertex_array_track);   // bind vao to GL_ARRAY_BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, m_vbuffer_track_pos);// bind vbo to GL_ARRAY_BUFFER
  //assign storage to buffer object
  glNamedBufferData(m_vbuffer_track_pos, sizeof(GLfloat)*4*1024, NULL, GL_STATIC_DRAW); //TODO
  ////// program
  //// shader and variables
  // create program
  m_shader_vertex_track   = createShader(GL_VERTEX_SHADER,   default_vertex_glsl_track);
  m_shader_fragment_track = createShader(GL_FRAGMENT_SHADER, default_fragment_glsl_track);
  m_program_track = glCreateProgram();
  glAttachShader(m_program_track, m_shader_vertex_track);
  glAttachShader(m_program_track, m_shader_fragment_track);
  glLinkProgram(m_program_track);
  // get attributes from program
  m_blockindex_geo_track = glGetUniformBlockIndex(m_program_track, "GeoData");
  m_blockindex_transform_track = glGetUniformBlockIndex(m_program_track, "TransformData");
  m_location_track_pos = glGetAttribLocation(m_program_track, "pos");
  glVertexAttribPointer(m_location_track_pos, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);  //TODO
  // connect attributes to buffer
  //// connect uniform block to bind point (ubuffer)
  glUniformBlockBinding(m_program_track, m_blockindex_geo_track, m_bindpoint_geo);
  glUniformBlockBinding(m_program_track, m_blockindex_transform_track, m_bindpoint_transform);
  //// connect vertex attribute to vertex array object (vbuffer)
  glBindVertexArray(m_vertex_array_track);
  glEnableVertexAttribArray(m_location_track_pos); //use currently bound vertex array object for the operation
  //////////////////////////////end of track////////////////////////////////////

  if(!js.IsNull()){
    updateGeometry(js);
  }

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
  ////// hit
  if(m_program_hit) {glDeleteProgram(m_program_hit); m_program_hit = 0;}
  if(m_shader_fragment_hit){glDeleteShader(m_shader_fragment_hit); m_shader_fragment_hit = 0;}
  if(m_shader_geometry_hit){glDeleteShader(m_shader_geometry_hit); m_shader_geometry_hit = 0;}
  if(m_shader_vertex_hit){glDeleteShader(m_shader_vertex_hit); m_shader_vertex_hit = 0;}
  if(m_vertex_array_hit){glDeleteVertexArrays(1, &m_vertex_array_hit); m_vertex_array_hit = 0;}
  if(m_vbuffer_hit_pos){glDeleteBuffers(1, &m_vbuffer_hit_pos); m_vbuffer_hit_pos = 0;}
  //

  ////// track
  if(m_program_track) {glDeleteProgram(m_program_track); m_program_track = 0;}
  if(m_shader_fragment_track){glDeleteShader(m_shader_fragment_track); m_shader_fragment_track = 0;}
  if(m_shader_vertex_track){glDeleteShader(m_shader_vertex_track); m_shader_vertex_track = 0;}
  if(m_vertex_array_track){glDeleteVertexArrays(1, &m_vertex_array_track); m_vertex_array_track = 0;}
  if(m_vbuffer_track_pos){glDeleteBuffers(1, &m_vbuffer_track_pos); m_vbuffer_track_pos = 0;}
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


void TelGL::updateGeometry(const JsonValue& js){
  if(!js.HasMember("geo")){
    std::fprintf(stderr, "unable to find \"geo\" key for detector goemerty from JS\n");
    return;
  }

  const auto &js_geo = js["geo"];
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

  glNamedBufferSubData(m_ubuffer_geo, 0, sizeof(TelGL::GeoDataGL)*m_counter_geo, &m_data_geo[0]);
  std::fprintf(stdout, "set up geometry data with %i layers\n", m_counter_geo);
}

void TelGL::drawDetectors(const JsonValue& js){
  if(js.HasMember("data")){
    const auto &js_data = js["data"];
    if(!js_data.IsArray() || !js_data.Size()){
      std::fprintf(stderr, "unable to to decode detectors from JS\n");
      return;
    }
    glUseProgram(m_program_tel);
    glBindVertexArray(m_vertex_array_tel);
    std::vector<GLint> gldata(js_data.Size());
    auto it = gldata.begin();
    for(const auto &js_det : js_data.GetArray()){
      *(it++) = js_det.GetInt();
    }
    glNamedBufferSubData(m_vbuffer_tel_id, 0, sizeof(GLint)*gldata.size(), gldata.data());
    glDrawArrays(GL_POINTS, 0, gldata.size());
  }
}

void TelGL::drawDetectors(){
  glUseProgram(m_program_tel);
  glBindVertexArray(m_vertex_array_tel);
  glNamedBufferSubData(m_vbuffer_tel_id, 0, sizeof(GLint)*m_counter_geo, m_data_tel_id);
  glDrawArrays(GL_POINTS, 0, m_counter_geo);
}

// TODO: change glsl from ivec3 to vec4
void TelGL::drawHits(const JsonValue& js){
  if(js.HasMember("data")){
    const auto &js_data = js["data"];
    if(!js_data.IsArray() || !js_data.Size()){
      std::fprintf(stderr, "unable to to decode hits from JS\n");
      return;
    }
    glUseProgram(m_program_hit);
    glBindVertexArray(m_vertex_array_hit);
    // TODO: glsl is vec3
    std::vector<GLfloat> gldata(4*js_data.Size());
    auto it = gldata.begin();
    for(const auto &js_hit : js_data.GetArray()){
      *(it++) = js_hit[0].GetDouble();
      *(it++) = js_hit[1].GetDouble();
      *(it++) = js_hit[2].GetDouble();
      *(it++) = js_hit[3].GetDouble();
    }
    glNamedBufferSubData(m_vbuffer_hit_pos, 0, sizeof(GLfloat)*gldata.size(), gldata.data());
    glDrawArrays(GL_POINTS, 0, gldata.size()/4);
  }
}

void TelGL::drawTracks(const JsonValue& js){
  if(js.HasMember("data")){
    const auto &js_data = js["data"];
    if(!js_data.IsArray() || !js_data.Size()){
      std::fprintf(stderr, "unable to to decode tracks from JS\n");
      return;
    }
    glUseProgram(m_program_track);
    glBindVertexArray(m_vertex_array_track);
    for(const auto &js_track : js_data.GetArray()){
      std::vector<GLfloat> gldata(4*js_track.Size());
      auto it = gldata.begin();
      for(const auto &js_hit : js_track.GetArray()){
        *(it++) = js_hit[0].GetDouble();
        *(it++) = js_hit[1].GetDouble();
        *(it++) = js_hit[2].GetDouble();
        *(it++) = js_hit[3].GetDouble();
      }
      glNamedBufferSubData(m_vbuffer_track_pos, 0, sizeof(GLfloat)*gldata.size(), gldata.data());
      glDrawArrays(GL_LINE_STRIP, 0, gldata.size()/4);
    }
  }
}


JsonDocument TelGL::createJsonDocument(const std::string& str){
  std::istringstream iss(str);
  rapidjson::IStreamWrapper isw(iss);
  JsonDocument jsdoc;
  jsdoc.ParseStream(isw);
  if(jsdoc.HasParseError()){
    std::fprintf(stderr, "rapidjson error<%s> when parsing input data at offset %llu\n",
                 rapidjson::GetParseError_En(jsdoc.GetParseError()), jsdoc.GetErrorOffset());
    throw std::runtime_error("TelGL::createJsonDocument has ParseError\n");
  }
  return jsdoc;
}

JsonValue TelGL::createJsonValue(JsonAllocator& jsa, const std::string& str){
  JsonDocument jsdoc = createJsonDocument(str);
  JsonValue js;
  js.CopyFrom(jsdoc, jsa);
  return js;
}

std::string TelGL::readFile(const std::string& path){
  std::ifstream ifs(path);
  if(!ifs.good()){
    std::fprintf(stderr, "unable to raed file path<%s>\n", path);
    throw std::runtime_error("File open error\n");
  }
  std::string str;
  str.assign((std::istreambuf_iterator<char>(ifs) ),
             (std::istreambuf_iterator<char>()));
  return str;
}

void TelGL::printJsonValue(const JsonValue& o, bool pretty){
  rapidjson::StringBuffer sb;
  if(pretty){
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    o.Accept(w);
  }
  else{
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
    o.Accept(w);
  }
  rapidjson::PutN(sb, '\n', 1);
  std::fwrite(sb.GetString(), 1, sb.GetSize(), stdout);
}
