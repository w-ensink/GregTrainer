/*
  ==============================================================================

    GridDisplay.h
    Created: 14 Oct 2019 9:56:58pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

//==========================================================================
// The Grid to be used by the player to fill in their guess

class GridDisplayComponent final : public juce::Component,
                                   private juce::ValueTree::Listener {
public:
  GridDisplayComponent(juce::ValueTree &, int numColumns, int numRows,
                       const juce::StringArray &rowsText,
                       const juce::Array<int> &relativeNotes);

  ~GridDisplayComponent() override;

  //======================================================================

  enum class TileState {
    tileActive,
    tileInactive,
    tileWrongAnswer,
    tileRightAnswer
  };

  //======================================================================

  void paint(juce::Graphics &g) override;

  void resized() override;

  //======================================================================

  void setSpaceBetweenTiles(int space) noexcept;

  void setStateForTile(int column, int row, TileState on) noexcept;

  void setSetabilityTile(int column, int row, bool settable) noexcept;

  void setSetabilityColumn(int column, bool settable) noexcept;

  void setStateForTileInColumnWithThisRelativeNote(int column, int relativeNote,
                                                   TileState);

  void turnAllTilesOff() noexcept;

  int getRelativeNoteOfActiveTileInColumn(int column);

  //======================================================================

  int getNumRows() const noexcept;

  int getNumColumns() const noexcept;

  //======================================================================

private:
  class GridTileComponent;

  //======================================================================

  int spaceBetweenTiles, halfSpaceBetweenTiles;

  int numRows, numColumns;

  juce::ValueTree tree;

  juce::OwnedArray<juce::OwnedArray<GridTileComponent>> tiles;

  //======================================================================

  void initializeGridTiles(const juce::StringArray &, const juce::Array<int> &);
  void setDefaultColours();
  void setAllRowsInColumnInactiveExceptThisOne(int column, int row);
  auto getBoundsForTile(int column, int row) -> juce::Rectangle<int>;

  void valueTreePropertyChanged(juce::ValueTree &,
                                const juce::Identifier &) override;

  //======================================================================

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridDisplayComponent)
};
