#pragma once
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include "Exceptions.hpp"



#define log(msg,level) std::cout << level <<"| : "<<msg<<'\n';
#define logInfo(msg) log(msg,logLevelInfo);
#define logWarn(msg) log(msg,logLevelWarning);
#define logErr(msg) std::cerr<<logLevelErr<<": "<<__FILE__<<" at: "<< __LINE__ <<"\n Err message: \n"<<msg<<'\n';

constexpr int err_Fatal = -1;
const std::string logLevelInfo = "INFO";
const std::string logLevelWarn = "WARNING";
const std::string logLevelErr = "ERROR";


