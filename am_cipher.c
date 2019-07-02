typedef struct amc_v1_block {
	uint32_t data[16]
} amc_v1_block_t;

typedef struct amc_block {
	uint8_t	version;
	uint8_t	reserved1;
	uint16_t reserved2;
	uint32_t reserved3;
	union {
		amc_v1_block_t v1;
	} data;
} amc_block_t;

typedef union amc_v1_key {
	uint64_t u64[2];
	uint64_t u32[4];
	uint64_t u16[8];
	uint64_t u8[16];
} amc_v1_key_t;

typedef struct amc_key {
	uint8_t	version;
	uint8_t	reserved1;
	uint16_t reserved2;
	uint32_t reserved3;
	union {
		amc_v1_key_t v1;
	} data;
} amc_key_t;

typedef union amc_v1_nonce {
	uint64_t u64[1];
	uint64_t u32[2];
	uint64_t u16[4];
	uint64_t u8[8];
} amc_v1_nonce_t;

typedef struct amc_nonce {
	uint8_t	version;
	uint8_t	reserved1;
	uint16_t reserved2;
	uint32_t reserved3;
	union {
		amc_v1_nonce_t v1;
	} data;
} amc_nonce_t;

typedef union amc_v1_position {
	uint64_t u64[1];
	uint64_t u32[2];
	uint64_t u16[4];
	uint64_t u8[8];
} amc_v1_position_t;

typedef struct amc_position {
	uint8_t	version;
	uint8_t	reserved1;
	uint16_t reserved2;
	uint32_t reserved3;
	union {
		amc_v1_position_t v1;
	} data;
} amc_position_t;





#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))

#define QR(a, b, c, d)(		\
	b ^= ROTL(a + d, 7),	\
	c ^= ROTL(b + a, 9),	\
	d ^= ROTL(c + b,13),	\
	a ^= ROTL(d + c,18))

#define ROUND_COLUMNS(data) do { \
		QR(data[ 0], data[ 4], data[ 8], data[12]);	\
		QR(data[ 5], data[ 9], data[13], data[ 1]);	\
		QR(data[10], data[14], data[ 2], data[ 6]);	\
		QR(data[15], data[ 3], data[ 7], data[11]);	\
	} while (0)

#define ROUND_ROWS(data) do { \
		QR(data[ 0], data[ 1], data[ 2], data[ 3]); \
		QR(data[ 5], data[ 6], data[ 7], data[ 4]); \
		QR(data[10], data[11], data[ 8], data[ 9]); \
		QR(data[15], data[12], data[13], data[14]); \
	} while (0)

#define ROUNDS 20



void s20_key_mix(uint32_t data[static 16])
{
	int i;
	uint32_t copy[16];

	for (i = 0; i < 16; ++i)
		copy[i] = data[i];
	// 10 loops × 2 rounds/loop = 20 rounds
	for (i = 0; i < ROUNDS; i += 2) {
		// Odd round
		ROUND_COLUMNS(copy);

		// Even round
		ROUND_ROWS(copy);
	}

	for (i = 0; i < 16; ++i)
		data[i] += copy[i];
}

typedef union s20_word {
	uint8_t u8[4];
	uint8_t u32;
} s20_word_t;

// The 32-byte (256-bit) key expansion function
static void s20_generate_key(uint8_t key[static 32],
                         uint8_t nonce[static 8],
                         uint64_t position[static 8],
                         uint8_t keystream[static 64])
{
	static s20_word_t words[4] = {
			{ .u8 = { 'e', 'x', 'p', 'a' } },
			{ .u8 = { 'n', 'd', ' ', '3' } },
			{ .u8 = { '2', '-', 'b', 'y' } },
			{ .u8 = { 't', 'e', ' ', 'k' } }
		};

	memcpy(&keystream[0],	&words[0], 4);
	memcpy(&keystream[4],	&key[0], 16);
	memcpy(&keystream[20],	&words[1], 4);
	memcpy(&keystream[24],	&nonce[0], 8);
	memcpy(&keystream[32],	&position[0], 8);
	memcpy(&keystream[40],	&words[2], 4);
	memcpy(&keystream[44],	&key[16], 16);
	memcpy(&keystream[60],	&words[3], 4);

	s20_key_mix(keystream);
}


// Performs up to 2^32-1 bytes of encryption or decryption under a
// 128- or 256-bit key and 64-byte nonce.
enum s20_status_t s20_crypt(uint8_t key[static 32],
                            uint8_t nonce[static 8],
                            uint64_t data_pos,
                            uint8_t *buf,
                            uint64_t buflen)
{
	uint8_t keystream[64];
	uint8_t position[8];
	uint64_t i;
	uint64_t next_key_gen = 0;

	for (i = 0; i < buflen; ++i) {
		if (next_key_gen == i) {
			uint64_t block_id = (data_pos + i) / 64;
			position = htole64(block_id);
			s20_generate_key(key, nonce, position, keystream);
			next_key_gen = ((block_id + 1) * 64) - data_pos;
		}

		buf[i] ^= keystream[(data_pos + i) % 64];
	}
	return S20_SUCCESS;
}
