#include "Touhou06.h"

namespace Touhou06
{

Touhou06::Touhou06(PROCESSENTRY32W const& pe32) : TouhouMainGameBase(pe32)
{
	gameState2 = 0;
}

Touhou06::~Touhou06()
{
}

bool InvalidMenuStateForGame(int menuState)
{
	// Only certain menu states are valid for being in-game.
	// They are valid when the game is able to be started from them.
	// This is important because the 'check in menu' marker can be 0 on some inappropriate menus
	// 16 -> music room
	// 1, 2 -> main menu (i.e. on the demo)
	// 10 -> player data
	return menuState == 16 || menuState == 1 || menuState == 2 || menuState == 10;
}

void Touhou06::readDataFromGameProcess()
{
	state.gameState = GameState::Playing;
	state.stageState = StageState::Stage;

	// Character
	ReadProcessMemory(processHandle, (LPCVOID)CHARACTER, (LPVOID)&character, 1, NULL);
	switch (character)
	{
	default:
	case 0: state.character = Character::Reimu; break;
	case 1: state.character = Character::Marisa; break;
	}

	// Teammate
	ReadProcessMemory(processHandle, (LPCVOID)SUB_CHARACTER, (LPVOID)&characterSub, 1, NULL);
	switch (characterSub)
	{
	default:
	case 0: state.subCharacter = SubCharacter::A; break;
	case 1: state.subCharacter = SubCharacter::B; break;
	}

	// Difficulty
	ReadProcessMemory(processHandle, (LPCVOID)DIFFICULTY, (LPVOID)&difficulty, 1, NULL);
	switch (difficulty)
	{
	default:
	case 0: state.difficulty = Difficulty::Easy; break;
	case 1: state.difficulty = Difficulty::Normal; break;
	case 2: state.difficulty = Difficulty::Hard; break;
	case 3: state.difficulty = Difficulty::Lunatic; break;
	case 4: state.difficulty = Difficulty::Extra; break;
	}

	// Stage
	ReadProcessMemory(processHandle, (LPCVOID)STAGE, (LPVOID)&stage, 1, NULL);

	// Game state
	ReadProcessMemory(processHandle, (LPCVOID)GAME_STATE, (LPVOID)&gameState, 1, NULL);

	// Game state 2 (boss life)
	ReadProcessMemory(processHandle, (LPCVOID)GAME_STATE_2, (LPVOID)&gameState2, 1, NULL);

	// Read menu state
	ReadProcessMemory(processHandle, (LPCVOID)MENU_STATE, (LPVOID)&menuState, 4, NULL);

	char practiceFlag = 0;
	ReadProcessMemory(processHandle, (LPCVOID)PRACTICE_FLAG, (LPVOID)&practiceFlag, 1, NULL);

	// Are we in the menu?
	int aMenu;
	ReadProcessMemory(processHandle, (LPCVOID)CHECK_IN_MENU_VALUE, &aMenu, 4, NULL); // == 0 when we are in-game
	if (stage > 0 && aMenu == 0 && !InvalidMenuStateForGame(menuState))
	{
		// We are in-game
		char replayFlag = 0;
		ReadProcessMemory(processHandle, (LPCVOID)REPLAY_FLAG, (LPVOID)&replayFlag, 1, NULL);

		if (practiceFlag == 1)
		{
			state.gameState = GameState::StagePractice;
		}
		else if (replayFlag == 1)
		{
			state.gameState = GameState::WatchingReplay;
		}

		// Stage 1: Rumia (mid-boss)
		if (stage == 1 && (gameState == 6 || gameState == 7))
		{
			state.stageState = StageState::Midboss;
		}
		// Stage 1: Rumia (boss)
		else if (stage == 1 && (gameState >= 16))
		{
			state.stageState = StageState::Boss;
		}

		// Stage 2: Daiyousei
		else if (stage == 2 && gameState == 19)
		{
			state.stageState = StageState::Midboss;
		}
		// Stage 2: Cirno
		else if (stage == 2 && gameState >= 25)
		{
			state.stageState = StageState::Boss;
		}

		// Stage 3: Meiling (mid-boss and boss)
		else if (stage == 3 && gameState >= 16)
		{
			state.stageState = StageState::Boss;
		}

		// Stage 4: Koakuma
		else if (stage == 4 && (gameState == 0 && gameState2 > 0))
		{
			state.stageState = StageState::Midboss;
		}
		// Stage 4: Patchouli
		else if (stage == 4 && gameState > 0)
		{
			state.stageState = StageState::Boss;
		}

		// Stage 5: Sakuya (mid-boss and boss)
		else if (stage == 5 && gameState >= 17)
		{
			state.stageState = StageState::Boss;
		}

		// Stage 6: Sakuya (mid-boss)
		else if (stage == 6 && gameState == 13)
		{
			state.stageState = StageState::Midboss;
		}
		// Stage 6: Remilia (boss)
		else if (stage == 6 && gameState >= 19)
		{
			state.stageState = StageState::Boss;
		}

		// Stage 7 Patchouli (mid-boss):
		else if (stage == 7 && (gameState >= 18 && gameState <= 20))
		{
			state.stageState = StageState::Midboss;
		}
		// Stage 7 Flandre (boss):
		else if (stage == 7 && gameState != 0)
		{
			state.stageState = StageState::Boss;
		}
	}
	else
	{
		// We're in the menu
		state.gameState = GameState::MainMenu;

		switch (menuState)
		{
		default:
		case 0:
		case 1:
		case 2:
		case 8:
		case 14: state.mainMenuState = MainMenuState::TitleScreen; break;

		case 6:
		case 7:
		case 9:
		case 11: state.mainMenuState = (practiceFlag == 1) ? MainMenuState::StagePractice : MainMenuState::GameStart; break;

		case 17: state.mainMenuState = MainMenuState::StagePractice; break;

		case 12:
		case 13:
		case 15: state.mainMenuState = MainMenuState::Replays; break;

		case 10: state.mainMenuState = MainMenuState::PlayerData; break;

		case 16:
		{
			state.mainMenuState = MainMenuState::MusicRoom;

			ReadProcessMemory(processHandle, (LPCVOID)MUSIC_ROOM_TRACK, (LPVOID)&bgm, 4, NULL);
			break;
		}

		case 3: state.mainMenuState = MainMenuState::Options; break;
		}
	}

	// Read current game progress
	char lives = 0;
	ReadProcessMemory(processHandle, (LPCVOID)LIVES, (LPVOID)&lives, 1, NULL);
	state.lives = lives;

	char bombs = 0;
	ReadProcessMemory(processHandle, (LPCVOID)BOMBS, (LPVOID)&bombs, 1, NULL);
	state.bombs = bombs;

	char gameOvers = 0;
	ReadProcessMemory(processHandle, (LPCVOID)GAMEOVERS, (LPVOID)&gameOvers, 1, NULL);
	state.gameOvers = gameOvers;

	ReadProcessMemory(processHandle, (LPCVOID)SCORE, (LPVOID)&state.score, 4, NULL);
	state.score = (state.score - gameOvers) / 10;
}

std::string Touhou06::getMidbossName() const
{
	switch (stage)
	{
	case 1: return "露米娅";
	case 2: return "大妖精";
	case 3: return "红美铃";
	case 4: return "小恶魔";
	case 5:
	case 6: return "十六夜咲夜";
	case 7: return "帕秋莉·诺蕾姬";
	default: return "";
	}
}

std::string Touhou06::getBossName() const
{
	switch (stage)
	{
	case 1: return "露米娅";
	case 2: return "琪露诺";
	case 3: return "红美铃";
	case 4: return "帕秋莉·诺蕾姬";
	case 5: return "十六夜咲夜";
	case 6: return "蕾米莉亚·斯卡蕾特";
	case 7: return "芙兰朵露·斯卡蕾特";
	default: return "";
	}
}

std::string const& Touhou06::getBGMName() const
{
	return th06_musicNames[bgm];
}

}