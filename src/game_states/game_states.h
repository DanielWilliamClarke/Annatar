#ifndef GAME_STATES
#define GAME_STATES


enum class GameStates {
	MENU,
	PLAY,
	ECS_PLAY,  // NEW: ECS-based play state
	PAUSE,
	STOP,
	SCORES,
	CREDITS
};

#endif // GAME_STATES