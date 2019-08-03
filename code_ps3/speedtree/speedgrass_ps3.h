
namespace SpeedGrass
{
	void Update(float elapsedTime);
	void OnAreaLoad(CHashedString clumpName, void* fileData, size_t fileSize);
	bool OnAreaUnload(CHashedString clumpName);
	unsigned int GetVRAMFootprint(unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize);
}

