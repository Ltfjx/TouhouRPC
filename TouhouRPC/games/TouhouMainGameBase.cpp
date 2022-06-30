#include "TouhouMainGameBase.h"

#include "../Log.h"
#include "../TextOutput.h"



TouhouMainGameBase::TouhouMainGameBase(PROCESSENTRY32W const& pe32)
	: TouhouBase(pe32)
{}

std::string TouhouMainGameBase::getStageName() const
{
	if (stage <= 6)
	{
		std::string name = "Stage ";
		name.append(std::to_string(stage));
		return name;
	}
	else
	{
		return "Extra Stage";
	}
}

void TouhouMainGameBase::StateData::updateImportantData(StateData const& b)
{
	gameState = b.gameState;
	stageState = b.stageState;
	gameOvers = b.gameOvers;

	lives = b.lives;
	bombs = b.bombs;

	currentPhotoCount = b.currentPhotoCount;
	requiredPhotoCount = b.requiredPhotoCount;

	mainItemUses = b.mainItemUses;
	subItemUses = b.subItemUses;
}

bool TouhouMainGameBase::StateData::isImportantDataEqual(StateData const& b) const
{
	return gameState == b.gameState
		&& stageState == b.stageState
		&& gameOvers == b.gameOvers

		&& lives == b.lives
		&& bombs == b.bombs

		&& currentPhotoCount == b.currentPhotoCount
		&& requiredPhotoCount == b.requiredPhotoCount

		&& mainItemUses == b.mainItemUses
		&& subItemUses == b.subItemUses
	;
}

bool TouhouMainGameBase::stateHasChangedSinceLastCheck()
{
	bool changed = TouhouBase::stateHasChangedSinceLastCheck() || !prevState.isImportantDataEqual(state);
	prevState.updateImportantData(state);
	return changed;
}

void TouhouMainGameBase::setGameName(std::string & name) const
{
	switch (state.gameState)
	{
	case GameState::MainMenu:
	{
		switch (state.mainMenuState)
		{
		case MainMenuState::TitleScreen: name = "主菜单"; break;
		case MainMenuState::GameStart:
		case MainMenuState::GameStart_Custom: name = "准备中"; break;
		case MainMenuState::ExtraStart: name = "Extra准备中"; break;
		case MainMenuState::StagePractice: name = "选择练习面数"; break;
		case MainMenuState::SpellPractice: name = "选择练习符卡"; break;
		case MainMenuState::Replays: name = "选择replay"; break;
		case MainMenuState::PlayerData: name = "查看玩家数据"; break;
		case MainMenuState::Achievements: name = "查看成就"; break;
		case MainMenuState::AbilityCards: name = "查看卡牌"; break;
		case MainMenuState::MusicRoom: name = "音乐室："; break; // game info will specify track.
		case MainMenuState::Options: name = "修改设置"; break;
		case MainMenuState::Manual: name = "查看手册"; break;
		}
		break;
	}

	case GameState::GameOver:
	{
		std::string AppendText;
		
		if (TO_StateHasChanged) { FailRandom=rand() % 5; }
		switch (FailRandom) {
		case 0:AppendText = "寄"; break;
		case 1:AppendText = "这么菜就别打了"; break;
		case 2:AppendText = "下播，请"; break;
		case 3:AppendText = "菜逼"; break;
		case 4:AppendText = "打的还不如土豆"; break;
		}
		name = AppendText;
		break;
	}

	case GameState::Completed:
	{
		// Scene-based games: Completion of a scene
		name = getStageName();
		name.append(" 完成");
		break;
	}

	case GameState::Fail:
	{
		// Scene-based games: Failing completion of a scene
		name.append(" 寄");
		break;
	}

	case GameState::Ending:
	case GameState::StaffRoll:
	{
		name = ("Cleared with ");
		name.append(createFormattedScore());
		name.append(" points");
		break;
	}

	case GameState::SpellPractice:
	{
		name = ("Practicing a spell:"); // game info will specify spell.
		break;
	}

	case GameState::WatchingReplay:
	{
		name = ("Watching a replay");
		break;
	}

	case GameState::StagePractice:
	{
		name = ("Practicing ");
		name.append(getStageName());
		break;
	}
	case GameState::Playing:
	{
		name.append(getStageName());

		// normal play shows resources or score
		name.reserve(name.size() + 6 /* text characters */ + 12 /* max score size */);
		name.append(" - (");
		if (showScoreInsteadOfResources)
		{
			name.append(createFormattedScore());
		}
		else
		{
			name.append(std::to_string(state.lives));
			name.append("/");
			name.append(std::to_string(state.bombs));
		}
		name.append(")");
		break;
	}
	case GameState::Playing_CustomResources:
	{
		name.append(getStageName());

		// normal play shows resources or score
		name.reserve(name.size() + 6 /* text characters */ + 12 /* max score size */);
		name.append(" - (");
		if (showScoreInsteadOfResources)
		{
			name.append(createFormattedScore());
		}
		else
		{
			name.append(getCustomResources());
		}
		name.append(")");
		break;
	}
	}
}

void TouhouMainGameBase::setGameInfo(std::string & info) const
{
	switch (state.gameState)
	{
	case GameState::MainMenu:
	{
		if (state.mainMenuState == MainMenuState::MusicRoom)
		{
			info = getBGMName();
		}
		if (state.mainMenuState == MainMenuState::GameStart_Custom)
		{
			info = getCustomMenuResources();
		}
		break;
	}

	case GameState::WatchingReplay:
	case GameState::Ending:
	case GameState::StaffRoll:
	case GameState::GameOver:
	case GameState::Fail:
	{
		break;
	}

	case GameState::Completed:
	{
		info = "Cleared with ";
		info.append(createFormattedScore());
		info.append(" points");
		break;
	}

	case GameState::SpellPractice:
	{
		info = getSpellCardName();
		break;
	}

	case GameState::Playing:
	case GameState::Playing_CustomResources:
	case GameState::StagePractice:
	{
		switch (state.stageState)
		{
		case StageState::Stage:
		{
			break;
		}
		case StageState::Midboss:
		{
			info = "道中Boss：";
			info.append(getMidbossName());
			break;
		}

		case StageState::Boss:
		{
			info = "Boss：";
			info.append(getBossName());
			break;
		}
		}
		break;
	}


	}
}

void TouhouMainGameBase::setLargeImageInfo(std::string & icon, std::string & text) const
{
	icon.clear(), text.clear();
	if (shouldShowCoverIcon())
	{
		icon.append("cover");
		return;
	}

	text = "机体：";

	switch (state.character)
	{
	case Character::Reimu:
	{
		icon.append("reimu"), text.append("博丽灵梦");
		break;
	}
	case Character::Marisa:
	{
		icon.append("marisa"), text.append("雾雨魔理沙");
		break;
	}
	case Character::Sakuya:
	{
		icon.append("sakuya"), text.append("十六夜咲夜");
		break;
	}
	case Character::Sanae:
	{
		icon.append("sanae"), text.append("东风谷早苗");
		break;
	}
	case Character::Youmu:
	{
		icon.append("youmu"), text.append("魂魄妖梦");
		break;
	}
	case Character::Reisen:
	{
		icon.append("reisen"), text.append("铃仙·优昙华院·因幡");
		break;
	}
	case Character::Cirno:
	{
		icon.append("cirno"), text.append("琪露诺");
		break;
	}
	case Character::Aya:
	{
		icon.append("aya"), text.append("射命丸文");
		break;
	}

	// IN Teams
	case Character::Border:
	{
		icon.append("border"), text.append("幻想结界");
		break;
	}
	case Character::Magic:
	{
		icon.append("magic"), text.append("禁咒咏唱");
		break;
	}
	case Character::Scarlet:
	{
		icon.append("scarlet"), text.append("梦幻红魔");
		break;
	}
	case Character::Nether:
	{
		icon.append("nether"), text.append("幽冥住人");
		break;
	}
	
	// IN Solo
	case Character::Yukari:
	{
		icon.append("yukari"), text.append("八云紫");
		break;
	}
	case Character::Alice:
	{
		icon.append("alice"), text.append("爱丽丝");
		break;
	}
	case Character::Remilia:
	{
		icon.append("remilia"), text.append("蕾米莉亚");
		break;
	}
	case Character::Yuyuko:
	{
		icon.append("yuyuko"), text.append("西行寺幽幽子");
		break;
	}

	// PoFV
	case Character::Lyrica:
	{
		icon.append("lyrica"), text.append("莉莉卡");
		break;
	}
	case Character::Merlin:
	{
		icon.append("merlin"), text.append("梅露兰");
		break;
	}
	case Character::Lunasa:
	{
		icon.append("lunasa"), text.append("露娜萨");
		break;
	}
	case Character::Mystia:
	{
		icon.append("mystia"), text.append("米斯蒂娅");
		break;
	}
	case Character::Tewi:
	{
		icon.append("tewi"), text.append("因幡帝");
		break;
	}
	case Character::Yuuka:
	{
		icon.append("yuuka"), text.append("风见幽香");
		break;
	}
	case Character::Medicine:
	{
		icon.append("medicine"), text.append("梅蒂欣");
		break;
	}
	case Character::Komachi:
	{
		icon.append("komachi"), text.append("小野塚小町");
		break;
	}
	case Character::Eiki:
	{
		icon.append("eiki"), text.append("四季映姬");
		break;
	}

	// ISC
	case Character::Seija:
	{
		icon.append("seija"), text.append("鬼人正邪");
		break;
	}
	}

	switch (state.subCharacter)
	{
	case SubCharacter::None:
	{
		break;
	}
	case SubCharacter::Team:
	{
		text.append("组");
		break;
	}
	case SubCharacter::Solo:
	{
		text.append("");//如果yyc选择了单人自机（Solo），则直接显示机体名
		break;
	}
	case SubCharacter::AltColour:
	{
		icon.append("alt");
		break;
	}
	case SubCharacter::A_DistinctIcons:
	{
		icon.append("a");
		// fall through
	}
	case SubCharacter::A:
	{
		text.append(" A");
		break;
	}
	case SubCharacter::B_DistinctIcons:
	{
		icon.append("b");
		// fall through
	}
	case SubCharacter::B:
	{
		text.append(" B");
		break;
	}
	case SubCharacter::C:
	{
		text.append(" C");
		break;
	}

	// SA partners
	case SubCharacter::AndYukari:
	{
		text.append(" + 八云紫");
		break;
	}
	case SubCharacter::AndSuika:
	{
		text.append(" + 伊吹萃香");
		break;
	}
	case SubCharacter::AndAya:
	{
		text.append(" + 射命丸文");
		break;
	}
	case SubCharacter::AndAlice:
	{
		text.append(" + 爱丽丝");
		break;
	}
	case SubCharacter::AndPatchouli:
	{
		text.append(" + 帕秋莉");
		break;
	}
	case SubCharacter::AndNitori:
	{
		text.append(" + 河城荷取");
		break;
	}

	// HSiFS seasons
	case SubCharacter::Spring:
	{
		text.append(" (春)");
		break;
	}
	case SubCharacter::Summer:
	{
		text.append(" (夏)");
		break;
	}
	case SubCharacter::Fall:
	{
		text.append(" (秋)");
		break;
	}
	case SubCharacter::Winter:
	{
		text.append(" (冬)");
		break;
	}

	// WBaWC beasts
	case SubCharacter::Wolf:
	{
		text.append(" (野狼灵)");
		break;
	}
	case SubCharacter::Otter:
	{
		text.append(" (水獭灵)");
		break;
	}
	case SubCharacter::Eagle:
	{
		text.append(" (大鹫灵)");
		break;
	}
	}
}

void TouhouMainGameBase::setSmallImageInfo(std::string & icon, std::string & text) const
{
	icon.clear(), text.clear();
	if (shouldShowCoverIcon()) return;

	text = "难度：";
	switch (state.difficulty)
	{
	case Difficulty::NoDifficultySettings:
	{
		text.clear();
		break;
	}
	case Difficulty::Easy:
	{
		icon = "easy", text.append("Easy");
		break;
	}
	case Difficulty::Normal:
	{
		icon = "normal", text.append("Normal");
		break;
	}
	case Difficulty::Hard:
	{
		icon = "hard", text.append("Hard");
		break;
	}
	case Difficulty::Lunatic:
	{
		icon = "lunatic", text.append("Lunatic");
		break;
	}
	case Difficulty::Extra:
	{
		icon = "extra", text.append("Extra");
		break;
	}
	case Difficulty::Phantasm:
	{
		icon = "phantasm", text.append("Phantasm");
		break;
	}
	case Difficulty::Overdrive:
	{
		icon = "overdrive", text.append("Overdrive");
		break;
	}
	}
}

bool TouhouMainGameBase::shouldShowCoverIcon() const
{
	return state.gameState != GameState::Playing
		&& state.gameState != GameState::Playing_CustomResources
		&& state.gameState != GameState::Completed
		&& state.gameState != GameState::Fail
		&& state.gameState != GameState::StagePractice
		&& state.gameState != GameState::SpellPractice
		&& state.gameState != GameState::WatchingReplay
		;
}

std::string TouhouMainGameBase::createFormattedScore() const
{
	std::string scoreString = std::to_string((state.score * 10) + state.gameOvers);
	int insertPosition = scoreString.length() - 3; // Do NOT use size_t as it is unsigned and can't be properly tested in the loop after, causing std::out_of_range exceptions.
	while (insertPosition > 0)
	{
		scoreString.insert(insertPosition, ",");
		insertPosition -= 3;
	}
	return scoreString;
}