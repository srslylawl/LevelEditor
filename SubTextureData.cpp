#include "SubTextureData.h"


Rendering::SubTextureData::SubTextureData(int xOffset, int yOffset, int width, int height) : xOffset(xOffset), yOffset(yOffset), width(width), height(height) { assetId = AssetId::CreateNewAssetId(); }
Rendering::SubTextureData::SubTextureData(int xOffset, int yOffset, int width, int height, AssetId assetId) : xOffset(xOffset), yOffset(yOffset), width(width), height(height), assetId(assetId) {}
