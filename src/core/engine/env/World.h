#ifndef WORLD_H
#define WORLD_H

#include <glm/vec3.hpp>
#include <unordered_map>
#include <memory>
#include <vector>

const int CHUNK_SIZE = 16;
const int WORLD_HEIGHT = 256;

class Mesh;
using ChunkCoord = std::pair<int, int>;

struct pair_hash {
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);
		// Combine the two hashes
		return h1 ^ (h2 << 1); // XOR and shift
	}
};


enum FaceDirection {
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	FRONT,
	BACK
};

struct Block {
	bool isSolid;
	int type;
};

struct Chunk {
	bool changed;
	Block blocks[CHUNK_SIZE][WORLD_HEIGHT][CHUNK_SIZE];
};

class World {
private:
	std::unordered_map<ChunkCoord, Chunk, pair_hash> m_loadedChunks;

public:
	Chunk* getChunk(int chunkX, int chunkZ);

	void loadChunk(int chunkX, int chunkZ);

	void unloadChunk(int chunkX, int chunkZ);

	void updateChunk(int chunkX, int chunkZ, const Chunk& updatedChunk);

	bool isBlockFaceVisible(const Chunk& chunk, const int& x, const int& y, const int& z, const FaceDirection& faceDirection);

	Mesh* generateMeshForChunk(const Chunk& chunk);

	void addFaceToMesh(const glm::vec3& origin,	std::vector<float>& vertexBuffer, std::vector<unsigned int>& indexBuffer, const FaceDirection& faceDirection, unsigned int& currentIndexOffset);
};

#endif