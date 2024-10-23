#pragma once
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include "Exceptions.hpp"


//TODO implement time getter function
static const char* getTime() {
	return "";
}

#define log(msg,level) std::cout<< getTime <<" |"<<level <<"| : "<<msg;
#define logErr(msg) std::cerr<<"An error occurred in: "<<__FILE__<<" at: "<< __LINE__ <<"\n Err message: \n"<<msg;

constexpr int err_Fatal = -1;
const std::string loggingInfo = "INFO";
const std::string loggingWarn = "WARNING";
const std::string loggingErr = "ERROR";


