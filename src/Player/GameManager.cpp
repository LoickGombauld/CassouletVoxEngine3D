#include "../Player/GameManager.hpp"
#include "../Voxel/Voxel.hpp"

namespace Voxel{
	GameManager::GameManager()
	{
		initialize();
	}
	GameManager::~GameManager()
	{
		// Destructor implementation
	}

	void GameManager::initialize()
	{
		Block::BlockRegistry::initialize();	
	}
	void GameManager::update(float deltaTime)
	{
		// Update game state here
	}
}