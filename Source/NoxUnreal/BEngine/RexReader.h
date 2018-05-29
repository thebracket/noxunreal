// (c) 2016 - Present, Bracket Productions

#pragma once

#include "CoreMinimal.h"
#include "../ThirdParty/libnox/Includes/zlib.h"

struct RexTile
{
	uint32_t CharacterCode;
	uint8_t ForegroundRed;
	uint8_t ForegroundGreen;
	uint8_t ForegroundBlue;
	uint8_t BackgroundRed;
	uint8_t BackgroundGreen;
	uint8_t BackgroundBlue;
};

class RexTileLayer
{
public:
	TArray<TUniquePtr<RexTile>> Tiles;
	RexTileLayer() = default;
	RexTileLayer(int width, int height)
	{
		Tiles.AddUninitialized(width * height);
	}
	~RexTileLayer()
	{
		Tiles.Empty();
	}
};

class RexTileMap
{
private:
	unsigned int width;
	unsigned int height;
	unsigned int layerCount;

public:
	TArray<TUniquePtr<RexTileLayer>> Layers;

	RexTileMap(unsigned int width, unsigned int height, unsigned int layers)
	{
		this->width = width;
		this->height = height;
		this->layerCount = layers;

		Layers.AddUninitialized(this->layerCount);
		for (unsigned int i = 0; i < layerCount; i++)
		{
			this->Layers[i] = MakeUnique<RexTileLayer>(width, height);
		}
	}
	~RexTileMap() = default;
};

/**
 * 
 */
class NOXUNREAL_API RexReader
{
private:
	unsigned int layerCountOffset;

	gzFile filestream;

	bool disposed = false;
	int layerCount = -1;
	int layerSizes[2];

	int GetInt(gzFile& in, int position = -1);
	unsigned char GetChar(gzFile& in, int position = -1);
	void CheckDisposed();

public:
	RexReader() = default;
	~RexReader() = default;

	bool LoadFile(const char * filename);
	void Dispose();

	int GetLayerCount();
	int GetLayerWidth();
	int GetLayerHeight();

	RexTileMap* GetTileMap();
};
