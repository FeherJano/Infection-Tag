#pragma once
#include <unordered_map>
#include <string>


class messageSet {
public: 
	//FORMAL USAGE: the messageType enum always starts with mt_OK and ends with mt_NOK; if you want to add an extra message type, insert it before mt_NOK!
	const static enum messageType{ mt_OK,mt_invalidOp,mt_connReq,mt_NOK};
	messageSet() {
		init();
	}
	~messageSet() {
		msgPool.clear();
	}
	const std::string getMsg(messageType t){
		auto it = msgPool.find(t);
		if (it == msgPool.end()) {
			return msgPool[mt_invalidOp];
		}
	}

private:
	std::unordered_map<messageType, std::string> msgPool;
	void init() {
		for (int i = mt_OK; i < mt_NOK;i++) {
			std::string toAppend;
			switch (i)
			{
			case mt_OK:
				toAppend = "OK";
				break;
			case mt_invalidOp:
				toAppend = "invalidOp";
				break;
			case mt_connReq:
				toAppend = "connRequest";
				break;
			case mt_NOK:
				toAppend = "NOK";
				break;
			default:
				continue;
				break;
			}
			appendToPool(messageType(i), toAppend);
		}
	}
	void appendToPool(messageType t, std::string msg) {
		if (t > mt_NOK) return;
		msgPool[t] = msg;
	}
};