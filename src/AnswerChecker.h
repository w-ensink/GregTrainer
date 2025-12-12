/*
  ==============================================================================

    SourceCode.h
    Created: 30 Oct 2019 9:13:27pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "GridDisplayComponent.h"

//==============================================================================
// First implementation of answer checker

class AnswerChecker
{
public:
    AnswerChecker (GridDisplayComponent& grid) : grid (grid) {}
    
    void compareMelodyToGridState (Melody::Ptr engineMelody)
    {
        if (engineMelody != nullptr)
        {
            auto gridMelody = makeMelodyFromGridState();
            setTileStatesForWrongAnswer (gridMelody);
            setTileStatesForRightAnswer (engineMelody);
        }
    }
    
private:
    
    GridDisplayComponent& grid;
    
    Melody::Ptr makeMelodyFromGridState()
    {
        auto relativeNotes = Array<int>();
        
        for (int column = 0; column < grid.getNumColumns(); ++column)
        {
            relativeNotes.add (grid.getRelativeNoteOfActiveTileInColumn (column));
        }
        
        return Melody::createMelodyWithOnlyRelativeNotesInfo (relativeNotes);
    }
    
    void setTileStatesForWrongAnswer (Melody::Ptr engineMelody)
    {
        auto relativeNotes = engineMelody->getRelativeNotes();
        
        for (int c = 0; c < grid.getNumColumns(); ++c)
        {
            auto wrongAnswerState = GridDisplayComponent::TileState::tileWrongAnswer;
            grid.setStateForTileInColumnWithThisRelativeNote (c, relativeNotes[c], wrongAnswerState);
        }
    }
    
    void setTileStatesForRightAnswer (Melody::Ptr gridMelody)
    {
        auto relativeNotes = gridMelody->getRelativeNotes();
        
        for (int c = 0; c < grid.getNumColumns(); ++c)
        {
            auto rightAnswerState = GridDisplayComponent::TileState::tileRightAnswer;
            grid.setStateForTileInColumnWithThisRelativeNote (c, relativeNotes[c], rightAnswerState);
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnswerChecker)
    
};
