/*
  ==============================================================================

    Identifiers.h
    Created: 25 Oct 2019 3:49:50pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

// Global ValueTree Identifiers

#define DECLARE_ID(name) static inline const juce::Identifier name = #name

struct IDs final {
  DECLARE_ID(GlobalRoot);

  struct Grid final {
    DECLARE_ID(GridRoot);
    DECLARE_ID(GridState);
    DECLARE_ID(TileState);
    DECLARE_ID(TileSetable);
    DECLARE_ID(TileText);
    DECLARE_ID(TileRelativeNote);
    DECLARE_ID(TileActiveColour);
    DECLARE_ID(TileInactiveColour);
    DECLARE_ID(TileWrongColour);
    DECLARE_ID(TileRightColour);
    DECLARE_ID(TileMouseHoover);
    DECLARE_ID(TileMouseDown);
  };

  struct Engine final {
    DECLARE_ID(EngineRoot);
    DECLARE_ID(PlayState);
    DECLARE_ID(MelodyLength);
    DECLARE_ID(EngineMelody);
  };
};

#undef DECLARE_ID

// Because tile Identifiers should be publicly available, just like normal IDs

class GridTileIdentifierManager final {
public:
  static void initializeTileIdentifiers(int numColumnsToGenerate,
                                        int numRowsToGenerate) {
    tileIdentifiers.resize(numColumns);

    numColumns = numColumnsToGenerate;
    numRows = numRowsToGenerate;

    for (int column = 0; column < numColumns; ++column) {
      auto r = juce::Array<juce::Identifier>();

      for (int row = 0; row < numRows; ++row)
        r.add(generateIdentifierForIndex(column, row));

      tileIdentifiers.add(r);
    }
  }

  static juce::Identifier getIdentifierForIndex(int column, int row) {
    jassert(column < numColumns && row < numRows);

    return tileIdentifiers[column][row];
  }

  static std::pair<int, int>
  getIndexForIdentifier(juce::Identifier identifier) {
    auto array = juce::StringArray::fromTokens(identifier.toString(), "_", "");
    auto column = array[0].getIntValue();
    auto row = array[1].getIntValue();

    return {column, row};
  }

private:
  static inline juce::Array<juce::Array<juce::Identifier>> tileIdentifiers;
  static inline int numColumns;
  static inline int numRows;

  static juce::Identifier generateIdentifierForIndex(int column, int row) {
    juce::StringArray a{juce::String{column}, juce::String{row}};

    return a.joinIntoString("_");
  }
};
