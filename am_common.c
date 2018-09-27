#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "am_common.h"

uint64_t am_common_log_mask = 0;
int	am_common_log_fd = 0;

void log_init(char* filename)
{
	if (filename == NULL)
		return;

	am_common_log_fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0777);
	if (am_common_log_fd == -1) {
		am_common_log_fd = 0;
		err("Unable to open file '%s' for logging\n", filename);
	}
}

void log_close()
{
	log_flush();
	close(am_common_log_fd);
}

void print_binary(const char* msg, const void* buf, const uint64_t length)
{
	uint64_t i;
	uint8_t* p = (uint8_t*)buf;

	if (msg != NULL)
		log("%s, length %lu, ", msg, length);
	for (i = 0; i < length; i++)
		__AM_LOG_TXT("%02x", p[i]);
	__AM_LOG_TXT("\n");
}

void print_binary_mask(const uint64_t msk, const char* msg, const void* buff, const uint64_t length)
{
	if (!log_printable(msk))
		return;
	print_binary(msg, buff, length);
}

#include <ctype.h>
#define PRINT_LINE_SIZE 16
#define PRINT_LINE_SPACE_EVERY 8

#define PRINT_LINE_SPACES_PER_LINE ((PRINT_LINE_SIZE / PRINT_LINE_SPACE_EVERY) - 1)
#define PRINT_LINE_BINARY_SIZE ((3 * PRINT_LINE_SIZE) - 1 + PRINT_LINE_SPACES_PER_LINE)
#define PRINT_LINE_ASCII_SIZE (PRINT_LINE_SIZE + PRINT_LINE_SPACES_PER_LINE)

static inline void print_hex_spaces(uint64_t length)
{
	for (; length > 0; length--) {
		log(" ");
	}
}

static inline void print_hex_binary(const uint8_t* start, const uint64_t length)
{
	uint64_t count;
	uint64_t printed = 0;

	for (count = 0; count < length; count++) {
		if (count != 0) {
			if ((count % 8) == 0) {
				log(" ");
				printed++;
			}
			log(" ");
			printed++;
		}
		log("%02x", start[count]);
		printed += 2;
	}
	print_hex_spaces(PRINT_LINE_BINARY_SIZE - printed);
}

static inline void print_hex_ascii(const uint8_t* start, const uint64_t length)
{
	uint64_t count;
	uint64_t printed = 0;
	char c;

	for (count = 0; count < length; count++) {
		if (count != 0 && (count % 8) == 0) {
			log(" ");
			printed++;
		}
		c = start[count];
		log("%c", (isprint(c) ? c : '.'));
		printed++;
	}
	print_hex_spaces(PRINT_LINE_ASCII_SIZE - printed);
}

static inline void print_hex_line(const uint8_t* start, const uint64_t length)
{
	print_hex_binary(start, length);
	log(" (");
	print_hex_ascii(start, length);
	log(")\n");
}

void print_hex(const char* msg, const void* buf, const uint64_t length)
{
	const uint8_t* start = buf;
	uint64_t offset = 0;

	if (msg != NULL)
			log("%s, length %lu, ", msg, length);

	log("%s, length %lu\n", msg, length);
	while (length - offset >= PRINT_LINE_SIZE) {
		log("%05lx ", offset);
		print_hex_line(start + offset, PRINT_LINE_SIZE);
		offset += PRINT_LINE_SIZE;
	}
	if (length - offset > 0) {
		log("%05lx ", offset);
		print_hex_line(start, length - offset);
	}
}

void print_hex_mask(const uint64_t msk, const char* msg, const void* buff, const uint64_t length)
{
	if (!log_printable(msk))
		return;
	print_binary(msg, buff, length);
}

rc_t fd_write(const int fd, const void* buf, const uint64_t len) // Returns 0 on success
{
	return (write(fd, buf, len) == len ? RC_SUCCESS : RC_ERROR);
}

rc_t fd_read(const int fd, void* buf, const uint64_t olen) // Returns 0 on success
{
	uint8_t* b = buf;
	uint64_t len = olen;
	int ret;

	while (len > 0) {
		ret = read(fd, b, len);
		if (ret < 0) {
			err("Error reading, errno %d\n", errno);
			return RC_ERROR;
		}
		if (ret == 0) {
			err("Could not finish reading. Read %lu/%lu byeas\n", olen - len, olen);
			return RC_ERROR;
		}
		len -= ret;
		b += ret;
	}

	return RC_SUCCESS;
}

void file_write(const char* filename, const void* data, const uint64_t length)
{
	int fd;
	int rc;

	fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0777);
	if (fd < 0) {
		err("Failed to open file '%s'\n", filename);
		return;
	}

	rc = fd_write(fd, data, length);
	close(fd);
	if (rc) {
		err("Failed to write to file file '%s'\n", filename);
	}

	log("Data saved to '%s'\n", filename);
}

void* file_read(const char* filename, uint64_t* length)
{
	int fd;
	int rc;
	off_t file_size;
	uint8_t* buf;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		err("Failed to open file '%s'\n", filename);
		return NULL;
	}

	file_size = lseek(fd, 0, SEEK_END);
	if (file_size == -1) {
		err("Failed to obtain size of '%s'\n", filename);
		return NULL;
	}

	rc = lseek(fd, 0, SEEK_SET);
	if (file_size == -1) {
		err("Failed to jump to start of '%s'\n", filename);
		return NULL;
	}

	buf = malloc(file_size);
	if (buf == NULL) {
		err("Failed to obtain memory of %ld bytes\n", file_size);
		return NULL;
	}

	rc = fd_read(fd, buf, file_size);
	close(fd);
	if (rc) {
		free(buf);
		return NULL;
	}

	if (length != NULL)
		*length = file_size;
	log("Read %ld bytes from '%s'\n", file_size, filename);
	return buf;
}



static rc_t skt_connect_i4(const char* addr, const uint16_t port, socket_t* fd)
{
	struct sockaddr_in remote;
	int _fd;

	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr(addr);
	remote.sin_port = htons(port);

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0) {
		err("Failed to allocate socket (error %d)\n", errno);
		return RC_ERROR;
	}

	if (connect(_fd, (struct sockaddr*)&remote, sizeof(remote)) != 0) {
		err("Connect failed to remote socket at path '%s:%u' (error %d)\n", addr, port, errno);
		return RC_ERROR;
	}

	*fd = _fd;
	log("connected to '%s:%u'\n", addr, port);
	return RC_SUCCESS;
}

static rc_t skt_connect_i6(const char* addr, const uint16_t port, socket_t* fd)
{
	err("%s Not implemented!\n", __FUNCTION__);
	return RC_ERROR;
}

static rc_t skt_connect_ux(const char* addr, const uint16_t port, socket_t* fd)
{
	err("%s Not implemented!\n", __FUNCTION__);
	return RC_ERROR;
}

rc_t skt_connect(const sa_family_t fam, const char* addr, const uint16_t port, socket_t* fd)
{
	*fd = -1;

	switch (fam) {
	case AF_INET: return skt_connect_i4(addr, port, fd);
	case AF_INET6: return skt_connect_i6(addr, port, fd);
	case AF_UNIX: return skt_connect_ux(addr, port, fd);
	default: break;
	}

	err("Unsupported socket family %x\n", fam);
	return RC_ERROR;
}

static rc_t skt_listen_i4(const char* addr, const uint16_t port, socket_t* fd)
{
	int skt;
	int rc;
	struct sockaddr_in name;
	int enable = 1;

	skt = socket(AF_INET, SOCK_STREAM, 0);
	if (skt == -1) {
		err("Failed to obtain a socket (error %d)\n", errno);
		return RC_ERROR;
	}

	rc = setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	if (rc != 0) {
		err("Failed to set reuse address option on server socket (error %d)\n",	errno);
		goto close;
	}

	rc = setsockopt(skt, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
	if (rc != 0) {
		err(
				"Failed to set reuse port option on server socket (error %d)\n",
				errno);
		goto close;
	}

	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	rc = bind(skt, (struct sockaddr*) &name, sizeof(name));
	if (rc != 0) {
		err("Error binding to ANY:%u (error %d)\n", port, errno);
		goto close;
	}

	rc = listen(skt, 5);
	if (rc != 0) {
		err("Can't listen on server socket (error %d)\n", errno);
		goto close;
	}

	*fd = skt;
	return RC_SUCCESS;

close:
	close(skt);
	return RC_ERROR;
}

static rc_t skt_listen_i6(const char* addr, const uint16_t port, socket_t* fd)
{
	err("%s Not implemented!\n", __FUNCTION__);
	return RC_ERROR;
}

static rc_t skt_listen_ux(const char* addr, const uint16_t port, socket_t* fd)
{
	err("%s Not implemented!\n", __FUNCTION__);
	return RC_ERROR;
}

rc_t skt_listen(const sa_family_t fam, const char* addr, const uint16_t port, socket_t* fd)
{
	*fd = -1;

	switch (fam) {
	case AF_INET: return skt_listen_i4(addr, port, fd);
	case AF_INET6: return skt_listen_i6(addr, port, fd);
	case AF_UNIX: return skt_listen_ux(addr, port, fd);
	default: break;
	}

	err("Unsupported socket family %x\n", fam);
	return RC_ERROR;
}

rc_t skt_accept(const socket_t server, socket_t* client)
{
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	socket_t cskt;

	*client = -1;

	cskt = accept(server, (struct sockaddr*) &client_name, &client_name_len);
	if (cskt == -1) {
		err("Failed to accept socket (error %d)\n", errno);
		return RC_ERROR;
	}

	*client = cskt;
	return RC_SUCCESS;
}

void skt_disconnect(socket_t* fd)
{
	if (close(*fd))
		err("Failed to disconnect socket %d\n", *fd);
	*fd = -1;
}

/* strncpy that returns number of character copied, EXCLUDING null terminator */
uint64_t strlncpy(void* to, const void* from, uint64_t remaining)
{
	uint8_t* _to = to;
	const uint8_t* _from = from;

	while(remaining > 0) {
		*_to = *_from;
		if (*_from == '\0')
			break;
		_to += 1;
		_from += 1;
		remaining -= 1;

	}

	return _to - (uint8_t*)to;
}

