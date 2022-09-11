// LoadTEX.c
// Loads a Angel Game Engine TEX file for OpenGL

#include "GLTools.h"
#include <stdio.h>

// Define targa header.
#pragma pack(1)
typedef struct
{
	u_short Width;
	u_short Height;
	u_short Type;
	u_short Mips;
	u_short _Unknown;
	u_char Flags1;
	u_char Flags2;
	u_char Flags3;
	u_char Flags4;
} AGETexHeader;
#pragma pack(8)

enum AGETexTypeID
{
	TEX_P8 = 1,
	TEX_PA8 = 14,
	TEX_P4_MC = 15,
	TEX_PA4_MC = 16,
	TEX_RGB888 = 17,
	TEX_RGBA8888 = 18,
};

struct AGETexColorMapEntryA8
{
	u_char blue;
	u_char green;
	u_char red;
	u_char alpha;
};


GLbyte* texReadBuffer(FILE* pFile, int size)
{
	GLbyte* pBits = (GLbyte*)malloc(size * sizeof(GLbyte));
	if (pBits == NULL)
		return NULL;

	if (fread(pBits, size, 1, pFile) != 1)
	{
		free(pBits);
		return NULL;
	}
	return pBits;
}


////////////////////////////////////////////////////////////////////
// Allocate memory and load targa bits. Returns pointer to new buffer,
// height, and width of texture, and the OpenGL format of data.
// Call free() on buffer when finished!
// This only works on pretty vanilla targas... 8, 24, or 32 bit color
// only, no palettes, no RLE encoding.
GLbyte* gltLoadTEX(const char* szFileName, GLint* iWidth, GLint* iHeight, GLint* iComponents, GLenum* eFormat)
{
	AGETexColorMapEntryA8 palette[256];
	FILE* pFile;				// File pointer
	AGETexHeader texHeader;		// TEX file header
	unsigned long lImageSize;	// Size in bytes of image
	short sDepth;				// Pixel depth;
	GLbyte* pBits = NULL;       // Pointer to bits
	GLbyte* pPixMap = NULL;
	int x = 0;
	int y = 0;

	// Default/Failed values
	*iWidth = 0;
	*iHeight = 0;
	*eFormat = GL_BGR_EXT;
	*iComponents = GL_RGB8;

	// Attempt to open the fil
	pFile = fopen(szFileName, "rb");
	if (pFile == NULL)
		return NULL;

	// Read in header (binary)
	fread(&texHeader, sizeof(AGETexHeader), 1, pFile);

	// Get width, height, and depth of texture
	*iWidth = texHeader.Width;
	*iHeight = texHeader.Height;
	
	switch (texHeader.Type)
	{
	case TEX_P8:
	case TEX_PA8:
		sDepth = 4;
		lImageSize = texHeader.Width * texHeader.Height * sDepth;

		pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
		if (pBits == NULL)
			return NULL;

		fread(palette, 256, sizeof(AGETexColorMapEntryA8), pFile);
		pPixMap = texReadBuffer(pFile, texHeader.Width * texHeader.Height);

		for (int i = 0; i < texHeader.Width * texHeader.Height; i++) 
		{
			u_char color_index = pPixMap[i];
			AGETexColorMapEntryA8 px = palette[color_index];

			pBits[((x * 4) + (y * texHeader.Width * 4) + 0)] = px.red;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 1)] = px.green;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 2)] = px.blue;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 3)] = px.alpha;

			if (++x >= texHeader.Width)
			{
				x = 0; y++;
			}
		}
		break;
	case TEX_P4_MC:
	case TEX_PA4_MC:
		sDepth = 4;
		lImageSize = texHeader.Width * texHeader.Height * sDepth;

		pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
		if (pBits == NULL)
			return NULL;

		fread(palette, 16, sizeof(AGETexColorMapEntryA8), pFile);
		pPixMap = texReadBuffer(pFile, (texHeader.Width * texHeader.Height) / 2);

		for (int i = 0; i < (texHeader.Width * texHeader.Height) / 2; i++)
		{
			u_char color_index1 = pPixMap[i] & 15;
			u_char color_index2 = pPixMap[i] >> 4 & 15;
			AGETexColorMapEntryA8 px1 = palette[color_index1];

			pBits[((x * 4) + (y * texHeader.Width * 4) + 0)] = px1.red;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 1)] = px1.green;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 2)] = px1.blue;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 3)] = px1.alpha;

			if (++x >= texHeader.Width)
			{
				x = 0; y++;
			}

			// second part
			AGETexColorMapEntryA8 px2 = palette[color_index2];

			pBits[((x * 4) + (y * texHeader.Width * 4) + 0)] = px2.red;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 1)] = px2.green;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 2)] = px2.blue;
			pBits[((x * 4) + (y * texHeader.Width * 4) + 3)] = px2.alpha;

			if (++x >= texHeader.Width)
				x = 0; y++;
		}
		break;
	case TEX_RGB888:
		sDepth = 3;
		lImageSize = texHeader.Width * texHeader.Height * sDepth;
		pBits = texReadBuffer(pFile, lImageSize);
		break;
	case TEX_RGBA8888:
		sDepth = 4;
		lImageSize = texHeader.Width * texHeader.Height * sDepth;
		pBits = texReadBuffer(pFile, lImageSize);
		break;
	default:
		return NULL;
	}

	// Set OpenGL format expected
	switch (sDepth)
	{
	case 3:     // Most likely case
		*eFormat = GL_BGR_EXT;
		*iComponents = GL_RGB8;
		break;
	case 4:
		*eFormat = GL_BGRA_EXT;
		*iComponents = GL_RGBA8;
		break;
	case 1:
		*eFormat = GL_LUMINANCE;
		*iComponents = GL_LUMINANCE8;
		break;
	};

	free(pPixMap);

	// Done with File
	fclose(pFile);

	// Return pointer to image data
	return pBits;
}
