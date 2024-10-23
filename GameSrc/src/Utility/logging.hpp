#pragma once
#include <iostream>
#include "Exceptions.hpp"

#define logErr(msg) std::cerr<<"An error occurred in: "<<__FILE__<<" at: "<< __LINE__ <<"\n Err message: \n"<<msg;
constexpr auto err_Fatal = -1;