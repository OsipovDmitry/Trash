#ifndef HDRLOADER_H
#define HDRLOADER_H

/***********************************************************************************
	Created:	17:9:2002
	FileName: 	hdrloader.h
	Author:		Igor Kravtchenko
	
	Info:		Load HDR image and convert to a set of float32 RGB triplet.
************************************************************************************/

#include <vector>

class HDRLoaderResult {
public:
	int width, height;
	// each pixel takes 3 float32, each component can be of any value...
    std::vector<float> cols;
};

class HDRLoader {
public:
	static bool load(const char *fileName, HDRLoaderResult &res);
};

#endif
