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

#include "TelescopeGL.hh"
#include <SFML/Window.hpp>

#include <chrono>
#include <thread>
#include <iostream>

#include "linenoise.h"
#include "myrapidjson.h"

#include "JsonGenerator.hpp"

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
    exit(1);
  }

  TelescopeGL telgl;
  telgl.addTelLayer(0, 0, 0,   1, 0, 0, 0.028, 0.026, 1.0, 1024, 512);
  telgl.addTelLayer(0, 0, 30,  0, 1, 0, 0.028, 0.026, 1.0, 1024, 512);
  telgl.addTelLayer(0, 0, 60,  0, 0, 1, 0.028, 0.026, 1.0, 1024, 512);
  telgl.addTelLayer(0, 0, 120, 0.5, 0.5, 0, 0.028, 0.026, 1.0, 1024, 512);
  telgl.addTelLayer(0, 0, 150, 0, 0.5, 0.5, 0.028, 0.026, 1.0, 1024, 512);
  telgl.addTelLayer(0, 0, 180, 1, 0, 1, 0.028, 0.026, 1.0, 1024, 512);
  telgl.addTelLayer(0, 0, 250, 0.5, 0, 0.5, 0.028, 0.026, 1.0, 1024, 512);
  telgl.buildProgramTel();
  telgl.buildProgramHit();

  altel::JsonAllocator s_jsa;
  altel::JsonDocument doc(&s_jsa);
  altel::JsonGenerator gen(dataHit_path);

  bool running = true;
  uint64_t n_gl_frame = 0;
  sf::Event windowEvent;
  while (running)
  {
    sf::Event windowEvent;
    while (telgl.m_window->pollEvent(windowEvent))
    {
      switch (windowEvent.type)
      {
      case sf::Event::Closed:
        running = false;
        break;
        // key pressed
      case sf::Event::KeyPressed:{
        if(!gen){
          std::cout<< "end of events"<<std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          break;
        }
        telgl.clearHit();
        // const auto &js_ev = ev_it->GetObject();
        // const auto &js_layers = js_ev["layers"].GetArray();
        // std::cout<<"\n  "<<std::endl;
        // for(const auto& js_layer : js_layers){
        //  PrintJson(js_layer);
          // std::printf("recreated native cluster: ");
          telgl.addHit( 10/0.02924+0.1, 10/0.02688+0.1, 1 );
          std::printf("TODO:  add data to gl");
          std::printf(" \n");
        // }

        telgl.clearFrame();
        telgl.drawTel();
        telgl.drawHit();
        telgl.flushFrame();
        // ++ev_it;
        break;
      }
        // we don't process other types of events
      default:
        break;
      }
    }
  }
}

