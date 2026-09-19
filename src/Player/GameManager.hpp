namespace Voxel
{
	class GameManager
	{
	public:
		GameManager();
		~GameManager();
		void run();
	private:
		void initialize();
		void update(float deltaTime);
	};
}