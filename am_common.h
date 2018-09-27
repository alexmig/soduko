#ifndef __AM_COMMON__
#define __AM_COMMON__

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

enum {
	KB		= 1024,
	MB		= 1024 * KB,
	GB		= 1024 * MB,
	THOUSAND	= 1000,
	MILLION		= 1000 * THOUSAND,
	BILLION		= 1000 * MILLION,
};

typedef enum bool_e {
	false = 0,
	true = !false,
} bool_t;

typedef enum rc_e {
	RC_SUCCESS,
	RC_ERROR,
} rc_t;

typedef int socket_t;

// Useful macros

#define MMIN(a, b) (a > b ? b : a)
#define MMAX(a, b) (a > b ? a : b)

// Logging

extern uint64_t	am_common_log_mask;
extern int		am_common_log_fd;

#define __AM_LOG_TXT(fmt, args...)	dprintf(am_common_log_fd, fmt, ##args)
#define __AM_LOG_PFX(fmt, args...)	__AM_LOG_TXT("%s:%05d "fmt, __FUNCTION__, __LINE__, ##args)

#define log_printable(msk)		(!(msk) || ((msk) & am_common_log_mask))
#define outm(msk, fmt, args...)	{ if (log_printable((msk))) { __AM_LOG_TXT(fmt, ##args); }}
#define logm(msk, fmt, args...)	{ if (log_printable((msk))) { __AM_LOG_PFX(fmt, ##args); }}
#define wrnm(msk, fmt, args...)	{ if (log_printable((msk))) { __AM_LOG_PFX("WARNING: "fmt, ##args); }}
#define errm(msk, fmt, args...)	{ if (log_printable((msk))) { __AM_LOG_PFX("ERROR: "fmt, ##args); }}
#define log(fmt, args...)		{ __AM_LOG_TXT(fmt, ##args); }
#define wrn(fmt, args...)		{ __AM_LOG_TXT("WARNING: "fmt, ##args); }
#define err(fmt, args...)		{ __AM_LOG_TXT("ERROR: "fmt, ##args); }
#define log_flush()				fdatasync(am_common_log_fd)

#define log_set_mask(msk)	am_common_log_mask = (msk)
#define log_add_mask(msk)	am_common_log_mask |= (msk)
#define log_sub_mask(msk)	am_common_log_mask &= ~(msk)

void log_init(char* filename);
void log_close();

static inline uint64_t get_time()
{
	struct timeval __ft_time_timeval__;
	if (gettimeofday(&__ft_time_timeval__, NULL) == -1) {
		return 0;
	}
	return (((uint64_t)__ft_time_timeval__.tv_sec) * 1000000) + __ft_time_timeval__.tv_usec;
}

// Printouts

void print_binary(const char* msg, const void* buf, const uint64_t length);
void print_binary_mask(const uint64_t msk, const char* msg, const void* buff, const uint64_t length);

void print_hex(const char* msg, const void* buf, const uint64_t length);
void print_hex_mask(const uint64_t msk, const char* msg, const void* buff, const uint64_t length);

// fd io

rc_t fd_write(const int fd, const void* buf, const uint64_t len); // Returns 0 on success
rc_t fd_read(const int fd, void* buf, const uint64_t len); // Returns 0 on success

// file io

void file_write(const char* filename, const void* data, const uint64_t length);
void* file_read(const char* filename, uint64_t* length);

// Sockets (blocking)

rc_t skt_connect(const sa_family_t fam, const char* addr, const uint16_t port, socket_t* fd);
rc_t skt_listen(const sa_family_t fam, const char* addr, const uint16_t port, socket_t* fd);
rc_t skt_accept(const socket_t server, socket_t* client);
void skt_disconnect(socket_t* fd);
#define skt_write(skt, buf, len) fd_write((skt), (buf), (len))
#define skt_read(skt, buf, len) fd_read((skt), (buf), (len))

// Utillity

/* strncpy that returns number of character copied, EXCLUDING null terminator */
uint64_t strlncpy(void* to, const void* from, uint64_t remaining);

#endif // #ifndef __AM_COMMON__
