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


class TelFW(){
 public:
  static  void initializeGLFW(){
    glfwSetErrorCallback([](int error, const char* description){
                           fprintf(stderr, "Error: %s\n", description);});
    if (!glfwInit()){
      throw;
    }
  }

  static void terminateGLFW(){
    glfwTerminate();
  }

  static  GLFWwindow* createWindow(float sWinWidth, float sWinWidth, const std::string& title=""){
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* window = glfwCreateWindow((int)sWinWidth, (int)sWinHeight, title.c_str(), NULL, NULL);
    if (!window){
      glfwTerminate();
      throw;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height){
                                             glViewport(0, 0, width, height);});
    return window;
  }

  static void deleteWindow(GLFWwindow* window){
    glfwDestroyWindow(window);
  }

  template<typename T>
    static std::future<int> startAsyncLoop(
                                           T* t,
                                           std::function<void(T*)> enterLoopHook,
                                           std::function<bool(T*, float, float)> beforeClearHook,
                                           std::function<void(T*)> drawHook){
    return std::async(std::launch::async, &TelFW::asyncLoop,
                      t, enterLoopHook, beforeClearHook, drawHook);
  }

  template<typename T>
    static int static asyncLoop(GLFWwindow* window,
                                T* t,
                                std::function<void(T*)> enterLoopHook,
                                std::function<bool(T*, float, float)> beforeClearHook,
                                std::function<void(T*)> drawHook
                                ){
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;
    enterLoopHook(t);
    while (!glfwWindowShouldClose(window)){
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
      int width;
      int height;
      glfwGetFramebufferSize(window, &width, &height);
      float currentWinRatio = width / (float) height;

      if(!beforeClearHook(t, deltaTime, currentWinRatio)) continue;

      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      drawHook(t);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }
    return 0;
  }
};


class TelFWTest{
public:
  std::string geometry_path;
  std::string data_path;
  JsonDocument jsd_trans;
  altel::TelGL telgl;
  glm::vec3 cameraPos;
  glm::vec3 worldCenter;
  glm::vec3 cameraUp;
  TelFWTest(const std::string& path_geometry, const std::string& path_data)
    :geometry_path(path_geometry), data_path(path_data), telgl(JsonValue())
  {
  }

  void enterLoopHook(){
    std::string jsstr_geo = altel::TelGL::readFile(geometry_path);
    JsonDocument jsd_geo;
    jsd_geo = altel::TelGL::createJsonDocument(jsstr_geo);
    jsd_trans = altel::TelGL::createTransformExample();
    telgl.updataGeometry(jsd_geo);

    cameraPos   = glm::vec3(0.0f, 0.0f,  -1000.0f);
    worldCenter = glm::vec3(0.0f, 0.0f,  0.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
  }

  bool beforeClearHook(float deltaTime, float currentWinRatio){
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      return false;
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
    return true;
  }

  void drawHook(){
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
  }
};
