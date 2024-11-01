#include "Channel.h"



Channel::Channel(string ch_name, size_t buf_size) {
	name_ = ch_name;
	buf_ = rbuf_create(buf_size);
}

Channel::~Channel() {
	Clean();
	rbuf_destroy(buf_);
}

int Channel::Write(void* buf, int len) {
    return rbuf_write(buf_, static_cast<unsigned char*>(buf), len);
}

int Channel::Read(void* buf, int len) {
	return rbuf_read(buf_, static_cast<unsigned char*>(buf), len);
}

int Channel::Clean() {
    rbuf_clear(buf_);

    return 0;
}

int Channel::Avail() {
	return rbuf_available(buf_);
}

int Channel::Used() {
	return rbuf_used(buf_);
}

bool Channel::Empty() {
	return rbuf_used(buf_) == 0;
}

bool Channel::Full() {
	return rbuf_available(buf_) == 0;
}