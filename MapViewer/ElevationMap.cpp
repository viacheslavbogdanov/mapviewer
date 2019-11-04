#include "ElevationMap.h"
#include <png.h>
#include <pngconf.h>
#include <assert.h>


bool LoadTerrariumElevationMap(const std::string& _Filename, unsigned int& _OutExtent, std::vector<float>& _OutElevationMap)
{
	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep* row_pointers;

	char header[8];    // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE* fp = fopen(_Filename.c_str(), "rb");
	if (!fp)
		return false; // File could not be opened for reading
	fread(header, 1, 8, fp);
	if (png_sig_cmp((png_const_bytep)header, 0, 8))
		return false; // File is not recognized as a PNG file

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		return false; // png_create_read_struct failed

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		return false; // png_create_info_struct failed

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for (int y = 0; y < height; y++)
	{
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
	}

	png_read_image(png_ptr, row_pointers);

	fclose(fp);

	if (width != height)
	{
		return false;
	}
	_OutExtent = width - 4;

	png_byte colorType = png_get_color_type(png_ptr, info_ptr);
	if (colorType == PNG_COLOR_TYPE_RGBA)
	{
		for (int y = 2; y < height - 2; y++) 
		{
			png_byte* row = row_pointers[y];
			for (int x = 2; x < width - 2; x++) 
			{
				png_byte* ptr = &(row[x * 4]);
				// R = ptr[0], G = ptr[1], B = ptr[3], A = ptr[4] (unused)
				// Elevation value decoding: (red * 256 + green + blue / 256) - 32768
				int elevValue = (ptr[0] * 256 + ptr[1] + ptr[3] / 256) - 32768;
				_OutElevationMap.push_back((float)elevValue);
			}
		}
	}
	else
	{
		return false; // incorrect color format
	}

	free(row_pointers);
	return true;
}


float GetElevation(const std::vector<float>& _ElevationMap, int _X, int _Y)
{
	assert(_X >= 0 && _X < 512 && _Y >= 0 && _Y < 512);
	assert(_ElevationMap.size() == 262144);
	return _ElevationMap[_Y * 512 + _X];
}


float GetElevation(const std::vector<float>& _ElevationMap, float _normalizedX, float _normalizedY)
{
	assert(_ElevationMap.size() == 262144);
	float fDerivative = 511.f / 8192.f;
	return _ElevationMap[(uint32_t)(_normalizedY * fDerivative) * 512 + (uint32_t)(_normalizedX * fDerivative)];
}
