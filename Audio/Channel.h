#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include "../Core/hlring/rbuf.h"


using std::string;

class Channel {
public:
	Channel(string ch_name, size_t buf_size = 19200 * 2);
	~Channel();

	// return actually writen data.
    int Write(void* buf, int len);
	// return actually read data.
	int Read(void* buf, int len);
    int Clean();
	int Avail();
	int Used();
	bool Empty();
	bool Full();

	string GetName() { return name_; }
	RBuf_t* GetBuf() { return buf_; }

protected:
    string		name_;
	RBuf_t*		buf_{ nullptr };
};
