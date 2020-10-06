#include "getopt.h"

#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <chrono>
#include <thread>
#include <regex>
#include <atomic>
#include <future>

#include <cstdio>
#include <csignal>

#include <chrono>
#include <thread>
#include <iostream>

#include "TelGL.hh"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linenoise.h"
#include "myrapidjson.h"

std::string LoadFileToString(const std::string& path){
  std::ifstream ifs(path);
  if(!ifs.good()){
    std::cerr<<"LoadFileToString:: ERROR, unable to load file<"<<path<<">\n";
    throw;
  }

  std::string str;
  str.assign((std::istreambuf_iterator<char>(ifs) ),
             (std::istreambuf_iterator<char>()));
  return str;
}

template<typename T>
static void PrintJson(const T& o){
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    o.Accept(w);
    rapidjson::PutN(sb, '\n', 1);
    std::fwrite(sb.GetString(), 1, sb.GetSize(), stdout);
}


static const std::string help_usage = R"(
Usage:
  -help              help message
  -verbose           verbose flag
  -dataHit  [PATH]   path to data file (input)
  -geometry [PATH]   path to geometry file (input)
)";


float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
float sWinWidth = 1000;
float sWinHeight = 480;
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  400.0f);
glm::vec3 worldCenter = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

int main(int argc, char **argv){
  int do_help = false;
  int do_verbose = false;
  struct option longopts[] =
    {
     { "help",       no_argument,       &do_help,      1  },
     { "verbose",    no_argument,       &do_verbose,   1  },
     { "dataHit",    required_argument, NULL,           'd' },
     { "geometry",   required_argument, NULL,           'g' },
     { 0, 0, 0, 0 }};

  std::string dataHit_path;
  std::string geometry_path;

  int c;
  opterr = 1;
  while ((c = getopt_long_only(argc, argv, "", longopts, NULL))!= -1) {
    switch (c) {
    case 'd':
      dataHit_path = optarg;
      break;
    case 'g':
      geometry_path = optarg;
      break;
      /////generic part below///////////
    case 0: /* getopt_long() set a variable, just keep going */
      break;
    case 1:
      std::fprintf(stderr,"case 1\n");
      exit(1);
      break;
    case ':':
      std::fprintf(stderr,"case :\n");
      exit(1);
      break;
    case '?':
      std::fprintf(stderr,"case ?\n");
      exit(1);
      break;
    default:
      std::fprintf(stderr, "case default, missing branch in switch-case\n");
      exit(1);
      break;
    }
  }

  if(do_help){
    std::fprintf(stdout, "%s\n", help_usage.c_str());
    exit(0);
  }

  if(geometry_path.empty() || dataHit_path.empty()){
    std::fprintf(stdout, "Error: input option.\n");
    std::fprintf(stdout, "%s\n", help_usage.c_str());
    // exit(1);
  }

  glfwSetErrorCallback([](int error, const char* description){
                         fprintf(stderr, "Error: %s\n", description);});

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow((int)sWinWidth, (int)sWinHeight, "OpenGL Telescope test", NULL, NULL);
  if (!window){ glfwTerminate(); exit(EXIT_FAILURE);}
  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height){
                                           sWinWidth  = width;
                                           sWinHeight = height;
                                           glViewport(0, 0, width, height);});

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
                               if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
                                 glfwSetWindowShouldClose(window, GLFW_TRUE);
                               }
                               if (key == GLFW_KEY_Q && action != GLFW_RELEASE)
                                 cameraPos -= (cameraPos - worldCenter) * (deltaTime * 0.1f);
                               if (key == GLFW_KEY_E && action != GLFW_RELEASE)
                                 cameraPos += (cameraPos - worldCenter) * (deltaTime * 0.1f);
                               //note: rotation does not work as expectation, BUT WORKS SOMEHOW
                               if (key == GLFW_KEY_A && action != GLFW_RELEASE)
                                 cameraPos -= glm::cross((cameraPos - worldCenter), glm::vec3(0.0f, 1.0f,  0.0f)) * (deltaTime * 2.0f);
                               if (key == GLFW_KEY_D && action != GLFW_RELEASE)
                                 cameraPos += glm::cross((cameraPos - worldCenter), glm::vec3(0.0f, 1.0f,  0.0f)) * (deltaTime * 2.0f);
                               if (key == GLFW_KEY_W && action != GLFW_RELEASE)
                                 cameraPos -= glm::cross((cameraPos - worldCenter), glm::vec3(1.0f, 0.0f,  0.0f)) * (deltaTime * 2.0f);
                               if (key == GLFW_KEY_S && action != GLFW_RELEASE)
                                 cameraPos += glm::cross((cameraPos - worldCenter), glm::vec3(1.0f, 0.0f,  0.0f)) * (deltaTime * 2.0f);
                             });

  /////////////////////////////////////////////
  std::string geometry_js_string = LoadFileToString(geometry_path);
  TelGL telgl(geometry_js_string,"","","");

  while (!glfwWindowShouldClose(window)){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    telgl.updateTransform(cameraPos[0],   cameraPos[1],   cameraPos[2],
                          worldCenter[0], worldCenter[1], worldCenter[2],
                          cameraUp[0],    cameraUp[1],    cameraUp[2],
                          60.0f,          0.1f,           2000.0f,
                          sWinWidth / sWinHeight);

    telgl.draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
