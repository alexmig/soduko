#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
		__AM_LOG_PFX("%s, length %lu, ", msg, length);
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

int fd_write(const int fd, const void* buf, const uint64_t len) // Returns 0 on success
{
	return (write(fd, buf, len) != len);
}

int fd_read(const int fd, void* buf, const uint64_t olen) // Returns 0 on success
{
	uint8_t* b = buf;
	uint64_t len = olen;
	int ret;

	while (len > 0) {
		ret = read(fd, b, len);
		if (ret < 0) {
			err("Error reading, errno %d\n", errno);
			return ret;
		}
		if (ret == 0) {
			err("Could not finish reading. Read %lu/%lu byeas\n", olen - len, olen);
			return len;
		}
		len -= ret;
		b += ret;
	}

	return 0;
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
