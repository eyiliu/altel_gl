#pragma once

#include <vector>
#include <memory>

#include "gl.h"
#include <glm/glm.hpp>

// namespace altel{
  class TelGL{
  public:
    struct GeoDataGL{
      GLfloat pos[4]  {0,0,0,0}; //vec3, pad
      GLfloat color[4]{0,1,0,0}; //vec3, pad
      GLfloat pitch[4]{0.028, 0.026, 1, 0}; //pitch x,y, thick z, pad
      GLint   npixel[4]{1024, 512,   1, 0};//pixe number x, y, z/1, pad
      GLfloat miss_alignment[16]{1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1 }; //mat4
    };
    constexpr static GLuint MAX_ID_SIZE{8};

    GLuint m_shader_vertex{0};
    GLuint m_shader_geometry{0};
    GLuint m_shader_fragment{0};
    GLuint m_program{0};
    GLuint m_vertex_array_object{0};

    GLint m_location_id{0};
    GLuint m_vbuffer_id{0};
    GLint m_data_id[MAX_ID_SIZE];

    GLint m_location_model{0};
    glm::mat4 m_data_model;

    GLint m_location_view{0};
    glm::mat4 m_data_view;

    GLint m_location_proj{0};
    glm::mat4 m_data_proj;

    GLuint m_ubuffer_geodata[MAX_ID_SIZE];
    GeoDataGL m_data_geodata[MAX_ID_SIZE];

    TelGL(const std::string& geo_js_string,
          const std::string& vertex_glsl,
          const std::string& geometry_glsl,
          const std::string& fragment_glsl
          );

    ~TelGL();

    void lookAt(float cameraX, float cameraY, float cameraZ,
                float centerX, float centerY, float centerZ,
                float upvectX, float upvectY, float upvectZ,
                float povHoriz, float nearDist, float farDist,
                float ratioWidthHigh
                );
    void draw();
  };

// }
