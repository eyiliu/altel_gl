#include "TelFW.hh"

size_t TelFW::s_count = 0;

TelFW::TelFW(float sWinWidth, float sWinHeight, const std::string& title){
  if(!s_count){
    initializeGLFW();
  }
  m_win = createWindow(sWinWidth, sWinHeight, title);
  s_count++;
}

TelFW::~TelFW(){
  stopAsync();
  deleteWindow(m_win);
  s_count--;
}

GLFWwindow* TelFW::get(){
  return m_win;
}


int TelFW::stopAsync(){
  m_runflag = false;
  if(m_fut.valid()){
    return m_fut.get();
  }
  return 0;
}

bool TelFW::checkifclose(){
  return glfwWindowShouldClose(m_win) || !m_runflag;
}

void TelFW::clear(){
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TelFW::flush(){
  glfwSwapBuffers(m_win);
  glfwPollEvents();
}

void TelFW::initializeGLFW(){
  std::cout<< "init glfw"<<std::endl;
  glfwSetErrorCallback([](int error, const char* description){
                         fprintf(stderr, "Error: %s\n", description);});
  if (!glfwInit()){
    throw;
  }
}

void TelFW::terminateGLFW(){
  glfwTerminate();
}

GLFWwindow* TelFW::createWindow(float sWinWidth, float sWinHeight, const std::string& title){
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

void TelFW::deleteWindow(GLFWwindow* window){
  glfwDestroyWindow(window);
}


