//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BpaServiceListener.h"
#include "Scheduler.h"

#include <signal.h>

#include <MeshVizXLM/mapping/MoMeshViz.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>

#ifdef USE_H264
#include <Service.h>
#include <ServiceSettings.h>
#else
#include <RemoteViz/Rendering/Service.h>
#include <RemoteViz/Rendering/ServiceSettings.h>
#endif

#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/program_options.hpp>

using namespace RemoteViz::Rendering;
namespace keywords = boost::log::keywords;
namespace options = boost::program_options;

bool running; 

void sighandler(int /*sig*/)
{
  running = false;
}

void initLogging()
{
  boost::log::add_common_attributes();

  boost::log::add_file_log(
    keywords::file_name = "log%N.txt",
    keywords::rotation_size = 32 * 1024 * 1024, // 32MB
    keywords::format = "[%TimeStamp% | %ThreadID%] %Message%",
    keywords::auto_flush = true);
}

struct Settings
{
  std::string datadir;
  std::string host;
  int port;
};

void logOptions(const Settings& settings)
{
  BOOST_LOG_TRIVIAL(trace) << "host = " << settings.host;
  BOOST_LOG_TRIVIAL(trace) << "port = " << settings.port;
  BOOST_LOG_TRIVIAL(trace) << "datadir = " << settings.datadir;
}

Settings initOptions(int argc, char** argv)
{
  options::options_description desc("Available options");
  desc.add_options()
    ("help", "show help message")
    ("host", options::value<std::string>()->default_value("auto"), "Set ip address")
    ("port", options::value<int>()->default_value(8081), "Set port number")
    ("datadir", options::value<std::string>()->default_value("."), "Root directory of data sets");

  options::variables_map vm;
  options::store(options::command_line_parser(argc, argv).options(desc).run(), vm);
  options::notify(vm);

  if (vm.count("help"))
  {
    std::cout << "Usage:\n" << desc << std::endl;
    exit(0);
  }

  Settings settings;
  settings.host = vm["host"].as<std::string>();
  settings.port = vm["port"].as<int>();
  settings.datadir = vm["datadir"].as<std::string>();

  logOptions(settings);

  return settings;
}

int main(int argc, char** argv)
{
  initLogging();
  auto options = initOptions(argc, argv);

  MoMeshViz::init();
  SoVolumeRendering::init();

  ServiceSettings settings;
  settings.setIP(options.host);
  settings.setPort(options.port);
  settings.setUsedExtensions(ServiceSettings::MESHVIZXLM);//
    //| ServiceSettings::VOLUMEVIZ 
    //| ServiceSettings::VOLUMEVIZLDM);

  Scheduler sched;
  sched.start();

  auto serviceListener = std::make_shared<BpaServiceListener>(sched);
  serviceListener->setDataDir(options.datadir);
  Service::instance()->addListener(serviceListener);

  BOOST_LOG_TRIVIAL(trace) << "Starting service ...";

  // Open the service by using the settings
  if(Service::instance()->open(&settings))
  {
    BOOST_LOG_TRIVIAL(trace) << "The render service was started successfully";

    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGINT, &sighandler);

    running = true;

    while (running)
    {
      Service::instance()->dispatch();
      sched.postProcess();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Close the service
    Service::instance()->close();
  }
  else
  {
    BOOST_LOG_TRIVIAL(error) << "Unable to start service";
  }

  MoMeshViz::finish();
  sched.stop();

  return 0;
}

