#include "global.h"
#include "ScreenEnding.h"
#include "SongManager.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "song.h"
#include "ProfileManager.h"
#include "ActorUtil.h"
#include "GameState.h"
#include "MemoryCardManager.h"
#include "RageLog.h"
#include "StyleDef.h"
#include "GameManager.h"
#include "SongUtil.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "StageStats.h"


#define SCROLL_DELAY		THEME->GetMetricF("ScreenEnding","ScrollDelay")
#define SCROLL_SPEED		THEME->GetMetricF("ScreenEnding","ScrollSpeed")
#define TEXT_ZOOM			THEME->GetMetricF("ScreenEnding","TextZoom")



CString GetStatsLineTitle( PlayerNumber pn, EndingStatsLine line )
{
	switch( line )
	{
	case CALORIES_TODAY:	return "Calories Today";
	case CURRENT_COMBO:		return "Current Combo";
	case PERCENT_COMPLETE_EASY:
	case PERCENT_COMPLETE_MEDIUM:
	case PERCENT_COMPLETE_HARD:
	case PERCENT_COMPLETE_CHALLENGE:
		// Ugly...
		{
			StepsType st = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
			CString sStepsType = GAMEMAN->NotesTypeToThemedString(st);
			if( GAMESTATE->IsCourseMode() )
			{
				CourseDifficulty cd = (CourseDifficulty)line;
				if( !GAMESTATE->IsCourseDifficultyShown(cd) )
					return "";
				CString sDifficulty = CourseDifficultyToThemedString(cd);
				return ssprintf( "%s %% Complete", sStepsType.c_str() );
			}
			else
			{
				Difficulty dc = (Difficulty)line;
				CString sDifficulty = DifficultyToThemedString(dc);
				return ssprintf( "%s %% Complete", sStepsType.c_str() );
			}
		}
	default:	ASSERT(0);	return "";
	}
}

CString GetStatsLineValue( PlayerNumber pn, EndingStatsLine line )
{
	Profile* pProfile = PROFILEMAN->GetProfile( pn );
	ASSERT( pProfile );

	switch( line )
	{
	case CALORIES_TODAY:		return pProfile->GetDisplayTotalCaloriesBurned();
	case CURRENT_COMBO:			return Commify( pProfile->m_iCurrentCombo );
	case PERCENT_COMPLETE_EASY:
	case PERCENT_COMPLETE_MEDIUM:
	case PERCENT_COMPLETE_HARD:
	case PERCENT_COMPLETE_CHALLENGE:
		// Ugly...
		{
			StepsType st = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
			CString sStepsType = GAMEMAN->NotesTypeToThemedString(st);
			if( GAMESTATE->IsCourseMode() )
			{
				CourseDifficulty cd = (CourseDifficulty)line;
				if( !GAMESTATE->IsCourseDifficultyShown(cd) )
					return "";
				CString sDifficulty = CourseDifficultyToThemedString(cd);
				return ssprintf( "%06.3f%%", pProfile->GetCoursesPercentComplete(st,cd)*100 );
			}
			else
			{
				Difficulty dc = (Difficulty)line;
				CString sDifficulty = DifficultyToThemedString(dc);
				return ssprintf( "%06.3f%%", pProfile->GetSongsPercentComplete(st,dc)*100 );
			}
		}
	default:	ASSERT(0);	return "";
	}
}


ScreenEnding::ScreenEnding( CString sClassName ) : ScreenAttract( sClassName, false/*dont reset GAMESTATE*/ )
{
	if( PREFSMAN->m_bScreenTestMode )
	{
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_1, false);
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_2, false);

		GAMESTATE->m_PlayMode = PLAY_MODE_REGULAR;
		GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_pCurSong = SONGMAN->GetRandomSong();
		GAMESTATE->m_pCurCourse = SONGMAN->GetRandomCourse();
		GAMESTATE->m_pCurSteps[PLAYER_1] = GAMESTATE->m_pCurSong->GetAllSteps()[0];
		GAMESTATE->m_pCurSteps[PLAYER_2] = GAMESTATE->m_pCurSong->GetAllSteps()[0];
		g_CurStageStats.pSteps[PLAYER_1] = GAMESTATE->m_pCurSteps[PLAYER_1];
		g_CurStageStats.pSteps[PLAYER_2] = GAMESTATE->m_pCurSteps[PLAYER_2];
		GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 2;
		GAMESTATE->m_PlayerOptions[PLAYER_2].m_fScrollSpeed = 2;
		GAMESTATE->m_iCurrentStageIndex = 0;
		GAMESTATE->m_PlayerOptions[PLAYER_1].ChooseRandomMofifiers();
		GAMESTATE->m_PlayerOptions[PLAYER_2].ChooseRandomMofifiers();

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = fmodf(f/100*4+.3f,1);
			g_CurStageStats.SetLifeRecord( PLAYER_1, fP1, f );
			g_CurStageStats.SetLifeRecord( PLAYER_2, 1-fP1, f );
		}
	
		g_CurStageStats.iActualDancePoints[PLAYER_1] = rand()%3;
		g_CurStageStats.iPossibleDancePoints[PLAYER_1] = 2;
		g_CurStageStats.iActualDancePoints[PLAYER_2] = rand()%2;
		g_CurStageStats.iPossibleDancePoints[PLAYER_2] = 1;
		g_CurStageStats.iCurCombo[PLAYER_1] = 0;
		g_CurStageStats.UpdateComboList( PLAYER_1, 0, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 1;
		g_CurStageStats.UpdateComboList( PLAYER_1, 1, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 50;
		g_CurStageStats.UpdateComboList( PLAYER_1, 25, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 250;
		g_CurStageStats.UpdateComboList( PLAYER_1, 100, false );

		g_CurStageStats.iTapNoteScores[PLAYER_1][TNS_MARVELOUS] = rand()%2;
		g_CurStageStats.iTapNoteScores[PLAYER_1][TNS_PERFECT] = rand()%2;
		g_CurStageStats.iTapNoteScores[PLAYER_1][TNS_GREAT] = rand()%2;
		g_CurStageStats.iTapNoteScores[PLAYER_2][TNS_MARVELOUS] = rand()%2;
		g_CurStageStats.iTapNoteScores[PLAYER_2][TNS_PERFECT] = rand()%2;
		g_CurStageStats.iTapNoteScores[PLAYER_2][TNS_GREAT] = rand()%2;

		g_vPlayedStageStats.clear();
	}


	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SongUtil::SortSongPointerArrayByTitle( arraySongs );

	FOREACH_PlayerNumber( p )
	{
		m_bWaitingForRemoveCard[p] = false;

		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		Profile* pProfile = PROFILEMAN->GetProfile( p );

		m_textPlayerName[p].LoadFromFont( THEME->GetPathToF("ScreenEnding player name") );
		m_textPlayerName[p].SetText( pProfile ? pProfile->GetDisplayName() : "NO CARD" );
		m_textPlayerName[p].SetName( ssprintf("PlayerNameP%d",p+1) );
		SET_XY_AND_ON_COMMAND( m_textPlayerName[p] );
		this->AddChild( &m_textPlayerName[p] );

		m_bWaitingForRemoveCard[p] = MEMCARDMAN->GetCardState(p)!=MEMORY_CARD_STATE_NO_CARD;

		if( pProfile == NULL )
			continue;	// don't show the stats lines
	
		FOREACH_EndingStatsLine( i )
		{
			m_Lines[i][p].title.LoadFromFont( THEME->GetPathToF("ScreenEnding stats title") );
			m_Lines[i][p].title.SetText( GetStatsLineTitle(p, i) );
			m_Lines[i][p].title.SetName( ssprintf("StatsTitleP%dLine%d",p+1,i+1) );
			SET_XY_AND_ON_COMMAND( m_Lines[i][p].title );
			this->AddChild( &m_Lines[i][p].title );
		
			m_Lines[i][p].value.LoadFromFont( THEME->GetPathToF("ScreenEnding stats value") );
			m_Lines[i][p].value.SetText( GetStatsLineValue(p, i) );
			m_Lines[i][p].value.SetName( ssprintf("StatsValueP%dLine%d",p+1,i+1) );
			SET_XY_AND_ON_COMMAND( m_Lines[i][p].value );
			this->AddChild( &m_Lines[i][p].value );
		}

		m_sprRemoveMemoryCard[p].SetName( ssprintf("RemoveCardP%d",p+1) );
		m_sprRemoveMemoryCard[p].Load( THEME->GetPathToG(ssprintf("ScreenEnding remove card P%d",p+1)) );
		SET_XY_AND_ON_COMMAND( m_sprRemoveMemoryCard[p] );
		this->AddChild( &m_sprRemoveMemoryCard[p] );
	}

	
	this->MoveToTail( &m_In );		// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );		// put it in the back so it covers up the stuff we just added

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("music scroll") );

	// Now that we've read the data from the profile, it's ok to Reset()
	GAMESTATE->Reset();

	float fSecsUntilBeginFadingOut = m_Background.GetLengthSeconds() - m_Out.GetLengthSeconds();
	if( fSecsUntilBeginFadingOut < 0 )
	{
		LOG->Warn( "Screen '%s' Out BGAnimation (%f seconds) is longer than Background BGAnimation (%f seconds); background BGA will be truncated",
			m_sName.c_str(), m_Out.GetLengthSeconds(), m_Background.GetLengthSeconds() );
		fSecsUntilBeginFadingOut = 0;
	}
	this->PostScreenMessage( SM_BeginFadingOut, fSecsUntilBeginFadingOut );
}

ScreenEnding::~ScreenEnding()
{
}

void ScreenEnding::Update( float fDeltaTime )
{
	ScreenAttract::Update( fDeltaTime );

	if( m_In.IsTransitioning() && m_Out.IsTransitioning() )
		return;

	FOREACH_PlayerNumber( p )
	{
		if( m_bWaitingForRemoveCard[p] )
		{
			m_bWaitingForRemoveCard[p] = MEMCARDMAN->GetCardState(p)!=MEMORY_CARD_STATE_NO_CARD;
			if( !m_bWaitingForRemoveCard[p] )
				m_sprRemoveMemoryCard[p].SetHidden( true );
		}
	}
}	

void ScreenEnding::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	bool bIsTransitioning = m_In.IsTransitioning() || m_Out.IsTransitioning();
	if( MenuI.IsValid() && !bIsTransitioning )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_START:
			SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
			break;
		}
	}

	ScreenAttract::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEnding::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenAttract::HandleScreenMessage( SM );
}

/*
 * (c) 2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
