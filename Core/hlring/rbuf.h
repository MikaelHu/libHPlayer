#pragma once
/**
* @file rbuf.h
*
* @brief Ring buffers
*
* Ringbuffer implementation store/access arbitrary binary data
*
* @todo allow to register i/o filters to be executed at read/write time
*
*/


#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

	typedef enum {
		RBUF_MODE_UNDEF = -1,
		RBUF_MODE_BLOCKING,
		RBUF_MODE_OVERWRITE,
		RBUF_MODE_NUM
	} eRBuf_Mode_t;

	/**
	* @brief Opaque structure representing a ringbuffer handler
	*/
	typedef struct RBuf RBuf_t;

	/**
	* @brief Create a new ringbuffer
	* @param size : The size of the ringbuffer (in bytes)
	* @return     : A pointer to an initialized rbuf_t structure
	*/
	RBuf_t *rbuf_create(int size);

	void rbuf_set_mode(RBuf_t *rbuf, eRBuf_Mode_t mode);

	eRBuf_Mode_t rbuf_get_mode(RBuf_t *rbuf);

	/**
	* @brief Skip the specified amount of bytes
	* @param rbuf  : A valid pointer to a rbuf_t structure
	* @param size :Tthe number of bytes to skip
	*/
	void rbuf_skip(RBuf_t *rbuf, int size);

	/**
	* @brief Read the specified amount of bytes from the ringbuffer
	* @param rbuf  : A valid pointer to a rbuf_t structure
	* @param out  : A valid pointer initialized to store the read data
	* @param size : The amount of bytes to read and copy to the memory
	*               pointed by 'out'
	* @return     : The amount of bytes actually read from the ringbuffer
	*/
	int rbuf_read(RBuf_t *rbuf, unsigned char *out, int size);
	/**
	* @brief Write the specified amount of bytes into the ringbuffer
	* @param rbuf  : A valid pointer to a rbuf_t structure
	* @param in   : A pointer to the data to copy into the ringbuffer
	* @param size : The amount of bytes to be copied
	* @return     : The amount of bytes actually copied into the ringbuffer
	* @note       : The ringbuffer may not fit the entire buffer to copy so
	*               the returned value might be less than the input 'size'.
	*               The caller should check for the returned value and retry
	*               writing the remainder once the ringbuffer has been emptied
	*               sufficiently
	*/
	int rbuf_write(RBuf_t *rbuf, unsigned char *in, int size);

	/**
	* @brief Returns the total size of the ringbuffer (specified at creation time)
	* @param rbuf  : A valid pointer to a rbuf_t structure
	* @return the total amount of bytes that can be stored in the rbuf
	*/
	int rbuf_size(RBuf_t *rbuf);

	/**
	* @brief Returns the amount of bytes stored into the ringbuffer
	*        and available for reading
	* @param rbuf  : A valid pointer to a rbuf_t structure
	* @return the amount of bytes stored into the ringbuffer
	*         and available for reading
	*/
	int rbuf_used(RBuf_t *rbuf);

	/**
	* @brief Returns the amount of space left in the ringbuffer for writing
	* @note equivalent to: rbuf_size() - rbuf_used()
	* @param rbuf  : A valid pointer to a rbuf_t structure
	* @return the amount of bytes which is still possible to write into the ringbuffer
	*         until some data is consumed by a rbuf_read() operation
	*/
	int rbuf_available(RBuf_t *rbuf);

	/**
	* @brief Scan the ringbuffer untill the specific byte is found
	* @param rbuf   : A valid pointer to a rbuf_t structure
	* @param octet : The byte to search into the ringbuffer
	* @return the offset to the specified byte, -1 if not found
	*/
	int rbuf_find(RBuf_t *rbuf, unsigned char octet);

	/**
	* @brief Read until a specific byte is found or maxsize is reached
	* @param rbuf     : A valid pointer to a rbuf_t structure
	* @param octet   : The byte to look for before stopping
	* @param out     : A valid pointer initialized to store the read data
	* @param maxsize : The maximum amount of bytes that can be copied to
	*                  the memory pointed by 'out'
	* @return        : The amount of bytes actually read from the ringbuffer
	*/
	int rbuf_read_until(RBuf_t *rbuf, unsigned char octet, unsigned char *out, int maxsize);

	int rbuf_move(RBuf_t *src, RBuf_t *dst, int len);

	int rbuf_copy(RBuf_t *src, RBuf_t *dst, int len);

	/**
	* @brief Clear the ringbuffer by eventually skipping all the unread bytes (if any)
	* @param rbuf : A valid pointer to a rbuf_t structure
	*/
	void rbuf_clear(RBuf_t *rbuf);

	/**
	* @brief Release all resources associated to the rbuf_t structure
	* @param rbuf : A valid pointer to a rbuf_t structure
	*/
	void rbuf_destroy(RBuf_t *rbuf);

#ifdef __cplusplus
}
#endif

// vim: tabstop=4 shiftwidth=4 expandtab:
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */