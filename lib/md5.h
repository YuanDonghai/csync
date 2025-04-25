/* Declaration of functions and data types used for MD5 sum computing
   library functions.
   Copyright (C) 1995-1997, 1999-2001, 2004-2006, 2008-2018 Free Software
   Foundation, Inc.
   This file is part of the GNU C Library.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

#ifndef _MD5_H
#define _MD5_H 1

#include <stdio.h>
#include <stdint.h>

#define MD5_DIGEST_SIZE 16
#define MD5_BLOCK_SIZE 64

#define BLOCKSIZE 32768

#ifdef WORDS_BIGENDIAN
# define SWAP(n) bswap_32 (n)
#else
# define SWAP(n) (n)
#endif

   /* These are the four functions used in the four steps of the MD5 algorithm
     and defined in the RFC 1321.  The first function is a little bit optimized
     (as found in Colin Plumbs public domain implementation).  */
     /* #define FF(b, c, d) ((b & c) | (~b & d)) */
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

static const unsigned char fillbuf[64] = { 0x80, 0 /* , 0, 0, ...  */ };

struct md5_ctx
{
   uint32_t A;
   uint32_t B;
   uint32_t C;
   uint32_t D;

   uint32_t total[2];
   uint32_t buflen;     /* ≥ 0, ≤ 128 */
   uint32_t buffer[32]; /* 128 bytes; the first buflen bytes are in use */
};

/*
 * The following three functions are build up the low level used in
 * the functions 'md5_stream' and 'md5_buffer'.
 */
 /* Compute MD5 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
void* md5_buffer(const char* buffer, size_t len, void* resblock);

/* Compute MD5 message digest for bytes read from STREAM.
   STREAM is an open file stream.  Regular files are handled more efficiently.
   The contents of STREAM from its current position to its end will be read.
   The case that the last operation on STREAM was an 'ungetc' is not supported.
   The resulting message digest number will be written into the 16 bytes
   beginning at RESBLOCK.  */
int md5_stream(FILE** stream, void* resblock);


/* Initialize structure containing state of computation.
   (RFC 1321, 3.3: Step 3)  */
void md5_init_ctx(struct md5_ctx* ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is necessary that LEN is a multiple of 64!!! */
void md5_process_block(const void* buffer, size_t len, struct md5_ctx* ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is NOT required that LEN is a multiple of 64.  */
void md5_process_bytes(const void* buffer, size_t len, struct md5_ctx* ctx);

/* Process the remaining bytes in the buffer and put result from CTX
   in first 16 bytes following RESBUF.  The result is always in little
   endian byte order, so that a byte-wise output yields to the wanted
   ASCII representation of the message digest.  */
void* md5_finish_ctx(struct md5_ctx* ctx, void* resbuf);


/* Put result from CTX in first 16 bytes following RESBUF.  The result is
   always in little endian byte order, so that a byte-wise output yields
   to the wanted ASCII representation of the message digest.  */
void* md5_read_ctx(const struct md5_ctx* ctx, void* resbuf);

static void set_uint32(char* cp, uint32_t v);
int check_file_with_md5(const char* file_name, char* md5_buf);
# endif