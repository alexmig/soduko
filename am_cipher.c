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





void salsa20_init(am_cipher_block_t* amcb, )
{
}

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d)(		\
	b ^= ROTL(a + d, 7),	\
	c ^= ROTL(b + a, 9),	\
	d ^= ROTL(c + b,13),	\
	a ^= ROTL(d + c,18))
#define ROUNDS 20

void salsa20_block(uint32_t out[16], uint32_t const in[16])
{
	int i;
	uint32_t x[16];

	for (i = 0; i < 16; ++i)
		x[i] = in[i];
	// 10 loops × 2 rounds/loop = 20 rounds
	for (i = 0; i < ROUNDS; i += 2) {
		// Odd round
		QR(x[ 0], x[ 4], x[ 8], x[12]);	// column 1
		QR(x[ 5], x[ 9], x[13], x[ 1]);	// column 2
		QR(x[10], x[14], x[ 2], x[ 6]);	// column 3
		QR(x[15], x[ 3], x[ 7], x[11]);	// column 4
		// Even round
		QR(x[ 0], x[ 1], x[ 2], x[ 3]);	// row 1
		QR(x[ 5], x[ 6], x[ 7], x[ 4]);	// row 2
		QR(x[10], x[11], x[ 8], x[ 9]);	// row 3
		QR(x[15], x[12], x[13], x[14]);	// row 4
	}
	for (i = 0; i < 16; ++i)
		out[i] = x[i] + in[i];
}
