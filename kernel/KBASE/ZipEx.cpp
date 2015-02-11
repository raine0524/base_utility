#include "stdafx.h"
#include "KBASE/ZipEx.h"
#include "zlib/zlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
unsigned char* KZipEx::Compress(void* pInput, unsigned long nInput, unsigned long* pnOutput, unsigned long nSuggest)
{
	// If we were given nSuggest, use it as the output buffer size,
	// otherwise call compressBound to set it
	// compressBound just uses math to guess, it doesn't look at the data
	*pnOutput = nSuggest ? nSuggest : compressBound(nInput);

	// Allocate a new buffer of pnOutput bytes
	unsigned char* pBuffer = new unsigned char[*pnOutput];

	// Compress the data at pInput into pBuffer, putting how many bytes it wrote under pnOutput
	if(::compress(						// Compress data from one buffer to another, returns Z_OK 0 false if it works
		pBuffer,						// The output buffer where ZLib can write compressed data
		pnOutput,						// Reads how much space it has there, writes how much space it used
		(const unsigned char*)pInput,	// The source buffer with data to compress
		nInput))						// The number of bytes there
	{
		// The compress function reported error
		delete[] pBuffer; // Delete our temporary buffer
		return NULL;      // Report error
	}

	// The pBuffer buffer is too big, make a new one exactly the right size, copy the data,
	// delete the first, and return the second
	unsigned char* pOutput = new unsigned char[*pnOutput];		// Allocate a new buffer exactly big enough to hold the bytes compress wrote
	memcpy(pOutput,pBuffer,*pnOutput );							// Copy the compressed bytes from the old buffer to the new one
	delete[] pBuffer;											// Delete the old buffer
	return pOutput;												// Return the new one
}

//---------------------------------------------------------------------------------------
unsigned char* KZipEx::Decompress(const void* pInput, unsigned long nInput, unsigned long* pnOutput, unsigned long nSuggest)
{
	// Guess how big the data will be decompressed, use nSuggest, 
	// or just guess it will be 6 times as big
	*pnOutput = nSuggest ? nSuggest : nInput * 6;

	// Allocate a buffer that big
	unsigned char* pBuffer = new unsigned char[ *pnOutput ];

	// Uncompress the data from pInput into pBuffer, writing how big it is now in pnOutput
	if( int nError = ::uncompress(		// Uncompress data
		pBuffer,						// Destination buffer where uncompress can write uncompressed data
		pnOutput,						// Reads how much space it has there, and writes how much space it used
		(const unsigned char*)pInput,   // Source buffer of compressed data
		nInput))						// Number of bytes there
	{
		// The uncompress function returned an error, delete the buffer we allocated and return error
		delete [] pBuffer;
		return NULL;
	}

	// The pBuffer buffer is bigger than necessary, move its bytes into one perfectly sized, and return it
	unsigned char* pOutput = new unsigned char[ *pnOutput ];     // Make a new buffer exactly the right size
	memcpy(pOutput,pBuffer,*pnOutput );			// Copy the data from the one that's too big
	delete[] pBuffer;							// Delete the one that's too big
	return pOutput;								// Return a pointer to the perfectly sized one
}

