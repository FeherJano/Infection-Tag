#pragma once
#include <exception>

//Make custom exceptions here

class noSuitableFontException : public std::exception {
	const char * myMessage = "No usable font file found! Check the filename spelling!";

public:
	inline const char* what(){
		return myMessage;
	}

};