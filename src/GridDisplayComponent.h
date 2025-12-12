/*
  ==============================================================================

    GridDisplay.h
    Created: 14 Oct 2019 9:56:58pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Utility.h"
#include "MelodyGenerator.h"




//==========================================================================
// The Grid to be used by the player to fill in their guess

class GridDisplayComponent final   : public  Component,
                                     private ValueTree::Listener
{
public:
    
    GridDisplayComponent (ValueTree&, int numColumns, int numRows,
                          const StringArray& rowsText, const Array<int>& relativeNotes);
    
    ~GridDisplayComponent();
    
    //======================================================================
    
    enum class TileState
    {
        tileActive,
        tileInactive,
        tileWrongAnswer,
        tileRightAnswer
    };
    
    //======================================================================
    
    void paint (Graphics& g) override;
    
    void resized() override;
    
    //======================================================================
    
    void setSpaceBetweenTiles (int space) noexcept;
    
    void setStateForTile (int column, int row, TileState on) noexcept;
    
    void setSetabilityTile (int column, int row, bool settable) noexcept;
    
    void setSetabilityColumn (int column, bool settable) noexcept;
    
    void setStateForTileInColumnWithThisRelativeNote (int column, int relativeNote, TileState);
    
    void turnAllTilesOff() noexcept;
    
    int getRelativeNoteOfActiveTileInColumn (int column);
    
    //======================================================================
    
    int getNumRows() const noexcept;
    
    int getNumColumns() const noexcept;
    
    //======================================================================
    
private:
    
    class GridTileComponent;

    //======================================================================
    
    int spaceBetweenTiles, halfSpaceBetweenTiles;
    
    int numRows, numColumns;
    
    ValueTree tree;
    
    OwnedArray<OwnedArray<GridTileComponent>> tiles;
    
    //======================================================================
    
    
    void initializeGridTiles (const StringArray&, const Array<int>&);
    void setDefaultColours();
    void setAllRowsInColumnInactiveExceptThisOne (int column, int row);
    auto getBoundsForTile (int column, int row) -> Rectangle<int>;
    
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    
    //======================================================================
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridDisplayComponent)
};
