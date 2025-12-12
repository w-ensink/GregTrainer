/*
  ==============================================================================

    GridDisplayComponent.cpp
    Created: 21 Oct 2019 5:22:58pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#include <juce_gui_extra/juce_gui_extra.h>

#include "GridDisplayComponent.h"
#include "Identifiers.h"
#include "MelodyGenerator.h"

//===============================================================================================
// VariantConverter for the TileState, to use for a CachedValue to stay in sync
// with ValueTree

template <>
class juce::VariantConverter<GridDisplayComponent::TileState> final {
public:
  using TileState = GridDisplayComponent::TileState;

  static juce::var toVar(const TileState &tileState) {
    if (tileState == TileState::tileActive)
      return "a";
    if (tileState == TileState::tileInactive)
      return "i";
    if (tileState == TileState::tileRightAnswer)
      return "r";
    if (tileState == TileState::tileWrongAnswer)
      return "w";

    return "i";
  }

  static TileState fromVar(const juce::var &state) {
    if (state == "a")
      return TileState::tileActive;
    if (state == "i")
      return TileState::tileInactive;
    if (state == "r")
      return TileState::tileRightAnswer;
    if (state == "w")
      return TileState::tileWrongAnswer;

    return TileState::tileInactive;
  }
};

//===============================================================================================
// Simple VariantConverter for Colour

template <> class juce::VariantConverter<juce::Colour> final {
public:
  static var toVar(const juce::Colour &colour) { return colour.toString(); }

  static juce::Colour fromVar(const juce::var &colourString) {
    return juce::Colour::fromString(colourString.toString());
  }
};

//===============================================================================================
// GridTileComponent represents a tile that the user can use to give their guess

class GridDisplayComponent::GridTileComponent final
    : public juce::Component,
      private juce::ValueTree::Listener {
public:
  GridTileComponent(juce::ValueTree &tree, const juce::Identifier &id)
      : valueTree(tree), tileIdentifier(id) {
    valueTree.addListener(this);

    tileText.referTo(valueTree, IDs::Grid::TileText, nullptr);
    mouseDownOnTile.referTo(valueTree, IDs::Grid::TileMouseDown, nullptr,
                            false);
    mouseHooveringOverTile.referTo(valueTree, IDs::Grid::TileMouseHoover,
                                   nullptr, false);
    tileIsSetable.referTo(valueTree, IDs::Grid::TileSetable, nullptr, true);
    tileState.referTo(valueTree, IDs::Grid::TileState, nullptr,
                      TileState::tileInactive);

    // they refer to their parent, because the colour settings should be the
    // same for all tiles
    if (auto gridTree = tree.getParent();
        gridTree.getType() == IDs::Grid::GridRoot) {
      tileActiveColour.referTo(gridTree, IDs::Grid::TileActiveColour, nullptr);
      tileInactiveColour.referTo(gridTree, IDs::Grid::TileInactiveColour,
                                 nullptr);
      tileRightAnswerColour.referTo(gridTree, IDs::Grid::TileRightColour,
                                    nullptr);
      tileWrongAnswerColour.referTo(gridTree, IDs::Grid::TileWrongColour,
                                    nullptr);
    }
  }

  ~GridTileComponent() {}

  //================================================================================

  void mouseDown(const juce::MouseEvent &) override {
    if (tileIsSetable)
      tileState = isTileOn() ? TileState::tileInactive : TileState::tileActive;

    mouseDownOnTile = true;
  }

  void mouseEnter(const juce::MouseEvent &) override {
    mouseHooveringOverTile = true;
  }

  void mouseExit(const juce::MouseEvent &) override {
    mouseHooveringOverTile = false;
  }

  void mouseUp(const juce::MouseEvent &) override { mouseDownOnTile = false; }

  //================================================================================

  void paint(juce::Graphics &g) override {
    g.setColour(getCurrentFillColour());
    g.fillRoundedRectangle(getLocalBounds().toFloat(), roundness);
    g.setColour(getCurrentTextColour());
    g.setFont(noteFont);
    g.drawText(tileText, getLocalBounds().reduced(10),
               juce::Justification::centred);
  }

  void resized() override {}

  //================================================================================

  bool isTileOn() const { return tileState == TileState::tileActive; }

private:
  juce::ValueTree valueTree;
  juce::Identifier tileIdentifier;

  juce::Font noteFont{"Arial", 30.0f, juce::Font::plain};

  float roundness{5.0f};

  juce::CachedValue<TileState> tileState;
  juce::CachedValue<juce::String> tileText;

  juce::CachedValue<juce::Colour> tileActiveColour;
  juce::CachedValue<juce::Colour> tileInactiveColour;
  juce::CachedValue<juce::Colour> tileRightAnswerColour;
  juce::CachedValue<juce::Colour> tileWrongAnswerColour;

  juce::CachedValue<bool> mouseHooveringOverTile;
  juce::CachedValue<bool> mouseDownOnTile;
  juce::CachedValue<bool> tileIsSetable;

  juce::Colour mouseHooverColour{juce::Colours::dimgrey};
  juce::Colour mouseDownColour{juce::Colours::white};

  //================================================================================

  juce::Colour getCurrentTextColour() const {
    return tileState == TileState::tileActive ? tileInactiveColour
                                              : tileActiveColour;
  }

  juce::Colour getCurrentFillColour() const {
    if (mouseDownOnTile)
      return mouseDownColour;
    if (mouseHooveringOverTile)
      return mouseHooverColour;

    return getColourForCurrentTileState();
  }

  juce::Colour getColourForCurrentTileState() const {
    if (tileState == TileState::tileActive)
      return tileActiveColour;
    if (tileState == TileState::tileRightAnswer)
      return tileRightAnswerColour;
    if (tileState == TileState::tileWrongAnswer)
      return tileWrongAnswerColour;

    return tileInactiveColour;
  }

  void valueTreePropertyChanged(juce::ValueTree &tree,
                                const juce::Identifier &id) override {
    repaint();
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridTileComponent)
};

//===============================================================================================
// Below all the implementations for GridDisplayComponent functions

GridDisplayComponent::GridDisplayComponent(
    juce::ValueTree &t, int numColumns, int numRows,
    const juce::StringArray &rowsText, const juce::Array<int> &relativeNotes)
    : numRows(numRows), numColumns(numColumns) {
  tree = juce::ValueTree{IDs::Grid::GridRoot};
  t.appendChild(tree, nullptr);

  jassert(numRows == rowsText.size() && relativeNotes.size() == numRows);

  GridTileIdentifierManager::initializeTileIdentifiers(numColumns, numRows);

  initializeGridTiles(rowsText, relativeNotes);
  setDefaultColours();

  setSpaceBetweenTiles(2);
  tree.addListener(this);
}

GridDisplayComponent::~GridDisplayComponent() {}

void GridDisplayComponent::paint(juce::Graphics &g) {
  g.setColour(juce::Colours::black);
  g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
}

juce::Rectangle<int> GridDisplayComponent::getBoundsForTile(int column,
                                                            int row) {
  auto [x, y, w, h] = getRectangleDimentions(getLocalBounds());

  w /= numColumns;
  h /= numRows;

  return {x + column * w + halfSpaceBetweenTiles,
          y + row * h + halfSpaceBetweenTiles, w - spaceBetweenTiles,
          h - spaceBetweenTiles};
}

void GridDisplayComponent::resized() {
  for (int column = 0; column < numColumns; ++column)
    for (int row = 0; row < numRows; ++row)
      tiles.getUnchecked(column)->getUnchecked(row)->setBounds(
          getBoundsForTile(column, row));
}

//===============================================================================================

void GridDisplayComponent::setSpaceBetweenTiles(int space) noexcept {
  spaceBetweenTiles = space;
  halfSpaceBetweenTiles = space / 2;
  repaint();
}

void GridDisplayComponent::setStateForTile(int column, int row,
                                           TileState state) noexcept {
  jassert(row < numRows && column < numColumns);

  auto tile = tree.getChildWithName(
      GridTileIdentifierManager::getIdentifierForIndex(column, row));

  tile.setProperty(IDs::Grid::TileState,
                   juce::VariantConverter<TileState>::toVar(state), nullptr);
}

void GridDisplayComponent::setSetabilityTile(int column, int row,
                                             bool setable) noexcept {
  jassert(row < numRows && column < numColumns);

  auto tile = tree.getChildWithName(
      GridTileIdentifierManager::getIdentifierForIndex(column, row));

  tile.setProperty(IDs::Grid::TileSetable, setable, nullptr);
}

void GridDisplayComponent::setSetabilityColumn(int column,
                                               bool settable) noexcept {
  jassert(column < numColumns);

  for (int row = 0; row < numRows; ++row)
    setSetabilityTile(column, row, settable);
}

//===============================================================================================

int GridDisplayComponent::getNumRows() const noexcept { return numRows; }

int GridDisplayComponent::getNumColumns() const noexcept { return numColumns; }

void GridDisplayComponent::turnAllTilesOff() noexcept {
  for (int column = 0; column < numColumns; ++column)
    for (int row = 0; row < numRows; ++row)
      setStateForTile(column, row, TileState::tileInactive);
}

void GridDisplayComponent::valueTreePropertyChanged(
    juce::ValueTree &tree, const juce::Identifier &id) {

  if (id == IDs::Grid::TileActiveColour ||
      id == IDs::Grid::TileInactiveColour || id == IDs::Grid::TileRightColour ||
      id == IDs::Grid::TileWrongColour)
    repaint();

  if (id == IDs::Grid::TileState)
    if (juce::VariantConverter<TileState>::fromVar(tree[id]) ==
        TileState::tileActive) {
      auto [column, row] =
          GridTileIdentifierManager::getIndexForIdentifier(tree.getType());
      setAllRowsInColumnInactiveExceptThisOne(column, row);
    }
}

void GridDisplayComponent::setAllRowsInColumnInactiveExceptThisOne(
    int column, int rowToLeaveActive) {
  for (int row = 0; row < numRows; ++row)
    if (row != rowToLeaveActive)
      setStateForTile(column, row, TileState::tileInactive);
}

void GridDisplayComponent::setStateForTileInColumnWithThisRelativeNote(
    int column, int relativeNote, TileState state) {
  for (int row = 0; row < numRows; ++row) {
    auto id = GridTileIdentifierManager::getIdentifierForIndex(column, row);
    auto tile = tree.getChildWithName(id);

    if (static_cast<int>(tile[IDs::Grid::TileRelativeNote]) == relativeNote)
      setStateForTile(column, row, state);
  }
}

int GridDisplayComponent::getRelativeNoteOfActiveTileInColumn(int column) {
  for (int row = 0; row < numRows; ++row) {
    auto tileType =
        GridTileIdentifierManager::getIdentifierForIndex(column, row);
    auto tile = tree.getChildWithName(tileType);

    if (juce::VariantConverter<TileState>::fromVar(
            tile[IDs::Grid::TileState]) == TileState::tileActive) {
      return tile[IDs::Grid::TileRelativeNote];
    }
  }

  return -1;
}

void GridDisplayComponent::initializeGridTiles(
    const juce::StringArray &rowsText, const juce::Array<int> &relativeNotes) {
  for (int column = 0; column < numColumns; ++column) {
    juce::OwnedArray<GridTileComponent> *col =
        new juce::OwnedArray<GridTileComponent>();

    for (int row = 0; row < numRows; ++row) {
      auto type = GridTileIdentifierManager::getIdentifierForIndex(column, row);
      juce::ValueTree childTile{type};
      tree.appendChild(childTile, nullptr);

      auto *tileptr = new GridTileComponent(childTile, type);

      col->set(row, tileptr);
      addAndMakeVisible(tileptr);

      childTile.setProperty(IDs::Grid::TileRelativeNote, relativeNotes[row],
                            nullptr);
      childTile.setProperty(IDs::Grid::TileText, rowsText[row], nullptr);
    }

    tiles.set(column, col);
  }
}

void GridDisplayComponent::setDefaultColours() {
  tree.setProperty(
      IDs::Grid::TileActiveColour,
      juce::VariantConverter<juce::Colour>::toVar(juce::Colours::gainsboro),
      nullptr);
  tree.setProperty(
      IDs::Grid::TileInactiveColour,
      juce::VariantConverter<juce::Colour>::toVar(juce::Colours::black),
      nullptr);
  tree.setProperty(
      IDs::Grid::TileRightColour,
      juce::VariantConverter<juce::Colour>::toVar(juce::Colours::green),
      nullptr);
  tree.setProperty(
      IDs::Grid::TileWrongColour,
      juce::VariantConverter<juce::Colour>::toVar(juce::Colours::red), nullptr);
}

//===============================================================================================
