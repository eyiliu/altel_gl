#pragma once

#include <cstdio>
#include <csignal>

#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <chrono>
#include <thread>
#include <regex>
#include <iostream>
#include <future>

#include <unistd.h>

#include "myrapidjson.h"

#include "gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "TelGL.hh"

class GLFWwindow;
class TelFW{
 public:
  TelFW(float sWinWidth, float sWinHeight, const std::string& title);
  ~TelFW();
  GLFWwindow* get();
  int stopAsync();

  template<typename T>
  bool startAsync(T* t,
                  std::function<int(T*, GLFWwindow*)> clearHook,
                  std::function<int(T*, GLFWwindow*)> drawHook
                  ){
    if(!m_fut.valid()){
      m_fut = std::async(std::launch::async, &TelFW::loopFun<T>,
                         this, t,  clearHook, drawHook);
    }
    return true;
  }

  template<typename T>
  int loopFun(T* t,
              std::function<int(T*, GLFWwindow*)> clearHook,
              std::function<int(T*, GLFWwindow*)> drawHook);

  static void initializeGLFW();
  static void terminateGLFW();
  static GLFWwindow* createWindow(float sWinWidth, float sWinHeight, const std::string& title);
  static void deleteWindow(GLFWwindow* window);
private:
  bool checkifclose();
  void clear();
  void flush();

private:
  bool m_runflag;
  std::future<int> m_fut;
  GLFWwindow* m_win;
  static size_t s_count;
};


template<typename T>
int TelFW::loopFun(T* t,
                   std::function<int(T*, GLFWwindow*)> clearHook,
                   std::function<int(T*, GLFWwindow*)> drawHook){
  m_runflag = true;
  while (!checkifclose()){
    std::cout<< ".";
    int ch = clearHook(t, m_win);
    if(ch==0) continue;
    if(ch>0){
    clear();
    }
    int dh = drawHook(t, m_win);
    flush();// todo: split flush and poll
  }
  m_runflag = false;
  std::cout<< "returned loop"<<std::endl;
  return 0;
}




class TelFWTest{
public:
  std::string geometry_path;
  std::string data_path;
  JsonDocument jsd_trans;
  altel::TelGL telgl;
  glm::vec3 cameraPos;
  glm::vec3 worldCenter;
  glm::vec3 cameraUp;
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  TelFWTest(const std::string& path_geometry, const std::string& path_data)
    :geometry_path(path_geometry), data_path(path_data), telgl(JsonValue())
  {
    std::string jsstr_geo = altel::TelGL::readFile(geometry_path);
    JsonDocument jsd_geo;
    jsd_geo = altel::TelGL::createJsonDocument(jsstr_geo);
    jsd_trans = altel::TelGL::createTransformExample();
    telgl.updateGeometry(jsd_geo);

    cameraPos   = glm::vec3(0.0f, 0.0f,  -1000.0f);
    worldCenter = glm::vec3(0.0f, 0.0f,  0.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
  }

  int clearHook(GLFWwindow* window){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);
    float currentWinRatio = width / (float) height;
    // if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    //   return 0;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      cameraPos -= (cameraPos - worldCenter) * (deltaTime * 1.0f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      cameraPos += (cameraPos - worldCenter) * (deltaTime * 1.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      cameraPos -= glm::cross((cameraPos - worldCenter), glm::vec3(0.0f, 1.0f,  0.0f)) * (deltaTime * 2.0f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      cameraPos += glm::cross((cameraPos - worldCenter), glm::vec3(0.0f, 1.0f,  0.0f)) * (deltaTime * 2.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      cameraPos -= glm::cross((cameraPos - worldCenter), glm::vec3(1.0f, 0.0f,  0.0f)) * (deltaTime * 2.0f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      cameraPos += glm::cross((cameraPos - worldCenter), glm::vec3(1.0f, 0.0f,  0.0f)) * (deltaTime * 2.0f);

    jsd_trans["trans"]["lookat"]["eye"]["x"]= cameraPos[0];
    jsd_trans["trans"]["lookat"]["eye"]["y"]= cameraPos[1];
    jsd_trans["trans"]["lookat"]["eye"]["z"]= cameraPos[2];
    jsd_trans["trans"]["lookat"]["center"]["x"]= worldCenter[0];
    jsd_trans["trans"]["lookat"]["center"]["y"]= worldCenter[1];
    jsd_trans["trans"]["lookat"]["center"]["z"]= worldCenter[2];
    jsd_trans["trans"]["lookat"]["up"]["x"]= cameraUp[0];
    jsd_trans["trans"]["lookat"]["up"]["y"]= cameraUp[1];
    jsd_trans["trans"]["lookat"]["up"]["z"]= cameraUp[2];
    jsd_trans["trans"]["persp"]["fov"]= 60;
    jsd_trans["trans"]["persp"]["ratio"]= currentWinRatio;
    jsd_trans["trans"]["persp"]["near"]= 0.1;
    jsd_trans["trans"]["persp"]["far"]= 2000;
    telgl.updateTransform(jsd_trans);
    return 1;
  }

  int drawHook(GLFWwindow* window){
    std::string jsstr_data = altel::TelGL::readFile(data_path);
    JsonDocument jsd_data = altel::TelGL::createJsonDocument(jsstr_data);

    if(jsd_data.HasMember("tracks")){
      telgl.drawTracks(jsd_data);
    }
    if(jsd_data.HasMember("hits")){
      telgl.drawHits(jsd_data);
    }
    if(jsd_data.HasMember("detectors")){
      telgl.drawDetectors(jsd_data);
    }
    else{
      telgl.drawDetectors();
    }
    return 1;
  }
};

