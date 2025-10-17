//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#ifndef BITMAP_H
#define BITMAP_H

#ifdef _WIN32
#pragma once
#endif


#include "bitmap/imageformat.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CUtlBuffer;


//-----------------------------------------------------------------------------
// A Bitmap
//-----------------------------------------------------------------------------
struct Bitmap_t
{
	Bitmap_t();
	~Bitmap_t();

	//
	// Accessors
	//
	inline int Width() const { return m_nWidth; }
	inline int Height() const { return m_nHeight; }
	inline ImageFormat Format() const { return m_ImageFormat; }
	inline unsigned char *GetBits() const { return m_pBits; }

	void Init( int nWidth, int nHeight, ImageFormat imageFormat );
	
	/// Return true if we have a valid size and buffer
	bool IsValid() const;

	unsigned char *GetPixel( int x, int y );
	

	/// Set this bitmap to be a cropped rectangle from the given bitmap.
	/// The source pointer can be NULL or point to this, which means to do
	/// the crop in place.
	void Crop( int x0, int y0, int nWidth, int nHeight, const Bitmap_t *pImgSource = NULL );

	int m_nWidth;
	int m_nHeight;
	ImageFormat m_ImageFormat;
	unsigned char *m_pBits;
};

inline Bitmap_t::Bitmap_t()
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_ImageFormat = IMAGE_FORMAT_UNKNOWN;
	m_pBits = NULL;
}

inline Bitmap_t::~Bitmap_t()
{
	if ( m_pBits )
	{
		delete[] m_pBits;
		m_pBits = NULL;
	}
}

inline void Bitmap_t::Init( int nWidth, int nHeight, ImageFormat imageFormat )
{
	if ( m_pBits )
	{
		delete[] m_pBits;
		m_pBits = NULL;
	}

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_ImageFormat = imageFormat;
	m_pBits = new unsigned char[ nWidth * nHeight * ImageLoader::SizeInBytes( m_ImageFormat ) ];
}

inline bool Bitmap_t::IsValid() const
{
	if ( m_nWidth <= 0 || m_nHeight <= 0 || m_pBits == NULL )
	{
		Assert( m_nWidth == 0 );
		Assert( m_nHeight == 0 );
		Assert( m_pBits == NULL );
		return false;
	}
	return true;
}

inline unsigned char *Bitmap_t::GetPixel( int x, int y )
{
	if ( !m_pBits )
		return NULL;

	int nPixelSize = ImageLoader::SizeInBytes( m_ImageFormat );
	return &m_pBits[ ( m_nWidth * y + x ) * nPixelSize ];
}

inline void Bitmap_t::Crop( int x0, int y0, int nWidth, int nHeight, const Bitmap_t *pImgSource )
{
#if 0
	// Check for cropping in place, then save off our data to a temp
	Bitmap_t temp;
	if ( pImgSource == this || !pImgSource )
	{
		temp.MakeLogicalCopyOf( *this, m_bOwnsBuffer );
		pImgSource = &temp;
	}

	// No source image?
	if ( !pImgSource->IsValid() )
	{
		Assert( pImgSource->IsValid() );
		return;
	}

	// Sanity check crop rectangle
	Assert( x0 >= 0 );
	Assert( y0 >= 0 );
	Assert( x0 + nWidth <= pImgSource->Width() );
	Assert( y0 + nHeight <= pImgSource->Height() );

	// Allocate buffer
	Init( nWidth, nHeight, pImgSource->Format() );

	// Something wrong?
	if ( !IsValid() )
	{
		Assert( IsValid() );
		return;
	}

	// Copy the data a row at a time
	int nRowSize = m_nWidth * m_nPixelSize;
	for ( int y = 0 ; y < m_nHeight ; ++y )
	{
		memcpy( GetPixel(0,y), pImgSource->GetPixel( x0, y + y0 ), nRowSize );
	}
#else
	// Can't crop
#endif
}

//-----------------------------------------------------------------------------
// Loads a bitmap from an arbitrary file: could be a TGA, PSD, or PFM.
// LoadBitmap autodetects which type, and returns it
//-----------------------------------------------------------------------------
enum BitmapFileType_t 
{ 
	BITMAP_FILE_TYPE_UNKNOWN = -1, 
	BITMAP_FILE_TYPE_PSD = 0, 
	BITMAP_FILE_TYPE_TGA, 
	BITMAP_FILE_TYPE_PFM, 
}; 

BitmapFileType_t LoadBitmapFile( CUtlBuffer &buf, Bitmap_t *pBitmap );


//-----------------------------------------------------------------------------
// PFM file loading related methods
//-----------------------------------------------------------------------------
bool PFMReadFileR32F( CUtlBuffer &fileBuffer, Bitmap_t &bitmap, float pfmScale );
bool PFMReadFileRGB323232F( CUtlBuffer &fileBuffer, Bitmap_t &bitmap, float pfmScale );
bool PFMReadFileRGBA32323232F( CUtlBuffer &fileBuffer, Bitmap_t &bitmap, float pfmScale );
bool PFMGetInfo_AndAdvanceToTextureBits( CUtlBuffer &pfmBuffer, int &nWidth, int &nHeight, ImageFormat &imageFormat );


#endif // BITMAP_H
