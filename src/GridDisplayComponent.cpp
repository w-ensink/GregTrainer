/*
  ==============================================================================

    GridDisplayComponent.cpp
    Created: 21 Oct 2019 5:22:58pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#include "GridDisplayComponent.h"
#include "Identifiers.h"
#include "MelodyGenerator.h"



//===============================================================================================
// VariantConverter for the TileState, to use for a CachedValue to stay in sync with ValueTree

template <>
class VariantConverter <GridDisplayComponent::TileState> final
{
public:
    using TileState = GridDisplayComponent::TileState;
    
    static var toVar (const TileState& tileState)
    {
        if (tileState == TileState::tileActive)      return "a";
        if (tileState == TileState::tileInactive)    return "i";
        if (tileState == TileState::tileRightAnswer) return "r";
        if (tileState == TileState::tileWrongAnswer) return "w";
        
        return "i";
    }
    
    static TileState fromVar (const var& state)
    {
        if (state == "a") return TileState::tileActive;
        if (state == "i") return TileState::tileInactive;
        if (state == "r") return TileState::tileRightAnswer;
        if (state == "w") return TileState::tileWrongAnswer;
        
        return TileState::tileInactive;
    }
};


//===============================================================================================
// Simple VariantConverter for Colour

template <>
class VariantConverter <Colour> final
{
public:
    static var toVar (const Colour& colour)
    {
        return colour.toString();
    }
    
    static Colour fromVar (const var& colourString)
    {
        return Colour::fromString (colourString.toString());
    }
};


//===============================================================================================
// GridTileComponent represents a tile that the user can use to give their guess

class GridDisplayComponent::GridTileComponent final     : public  Component,
                                                          private ValueTree::Listener
{
public:
    
    GridTileComponent (ValueTree& tree, const Identifier& id)  : valueTree (tree), tileIdentifier (id)
    {
        valueTree.addListener (this);
        
        tileText              .referTo (valueTree, IDs::Grid::TileText,        nullptr);
        mouseDownOnTile       .referTo (valueTree, IDs::Grid::TileMouseDown,   nullptr, false);
        mouseHooveringOverTile.referTo (valueTree, IDs::Grid::TileMouseHoover, nullptr, false);
        tileIsSetable         .referTo (valueTree, IDs::Grid::TileSetable,     nullptr, true);
        tileState             .referTo (valueTree, IDs::Grid::TileState,       nullptr, TileState::tileInactive);
        
        // they refer to their parent, because the colour settings should be the same for all tiles
        if (auto gridTree = tree.getParent(); gridTree.getType() == IDs::Grid::GridRoot)
        {
            tileActiveColour     .referTo (gridTree, IDs::Grid::TileActiveColour,   nullptr);
            tileInactiveColour   .referTo (gridTree, IDs::Grid::TileInactiveColour, nullptr);
            tileRightAnswerColour.referTo (gridTree, IDs::Grid::TileRightColour,    nullptr);
            tileWrongAnswerColour.referTo (gridTree, IDs::Grid::TileWrongColour,    nullptr);
        }
    }
    
    ~GridTileComponent() {}
    
    //================================================================================
    
    void mouseDown (const MouseEvent&) override
    {
        if (tileIsSetable)
           tileState = isTileOn() ? TileState::tileInactive : TileState::tileActive;
        
        mouseDownOnTile = true;
    }
    
    void mouseEnter (const MouseEvent&) override
    {
        mouseHooveringOverTile = true;
    }
    
    void mouseExit (const MouseEvent&) override
    {
        mouseHooveringOverTile = false;
    }
    
    void mouseUp (const MouseEvent&) override
    {
        mouseDownOnTile = false;
    }
    
    //================================================================================
    
    void paint (Graphics& g) override
    {
        g.setColour (getCurrentFillColour());
        g.fillRoundedRectangle (getLocalBounds().toFloat(), roundness);
        g.setColour (getCurrentTextColour());
        g.setFont (noteFont);
        g.drawText (tileText, getLocalBounds().reduced (10), Justification::centred);
    }
    
    
    void resized() override {}
    
    //================================================================================

    bool isTileOn() const
    {
        return tileState == TileState::tileActive;
    }
    
private:
    
    ValueTree valueTree;
    Identifier tileIdentifier;
    
    Font noteFont { "Arial", 30.0f, Font::plain };
    
    float roundness { 5.0f };
    
    CachedValue<TileState> tileState;
    CachedValue<String> tileText;
    
    CachedValue<Colour> tileActiveColour;
    CachedValue<Colour> tileInactiveColour;
    CachedValue<Colour> tileRightAnswerColour;
    CachedValue<Colour> tileWrongAnswerColour;
    
    CachedValue<bool> mouseHooveringOverTile;
    CachedValue<bool> mouseDownOnTile;
    CachedValue<bool> tileIsSetable;
    
    Colour mouseHooverColour { Colours::dimgrey };
    Colour mouseDownColour   { Colours::white   };
    
    //================================================================================
    
    Colour getCurrentTextColour() const
    {
        return tileState == TileState::tileActive ? tileInactiveColour : tileActiveColour;
    }
    
    Colour getCurrentFillColour() const
    {
        if (mouseDownOnTile)         return mouseDownColour;
        if (mouseHooveringOverTile)  return mouseHooverColour;
        
        return getColourForCurrentTileState();
    }
    
    Colour getColourForCurrentTileState() const
    {
        if (tileState == TileState::tileActive)      return tileActiveColour;
        if (tileState == TileState::tileRightAnswer) return tileRightAnswerColour;
        if (tileState == TileState::tileWrongAnswer) return tileWrongAnswerColour;
        
        return tileInactiveColour;
    }
    
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& id) override
    {
        repaint();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridTileComponent)
};


//===============================================================================================
// Below all the implementations for GridDisplayComponent functions

GridDisplayComponent::GridDisplayComponent (ValueTree& t, int numColumns, int numRows,
                                            const StringArray& rowsText, const Array<int>& relativeNotes) :
                                                                    numRows (numRows), numColumns (numColumns)
{
    tree = ValueTree {IDs::Grid::GridRoot};
    t.appendChild (tree, nullptr);
    
    jassert (numRows == rowsText.size() && relativeNotes.size() == numRows);
    
    GridTileIdentifierManager::initializeTileIdentifiers (numColumns, numRows);
   
    initializeGridTiles (rowsText, relativeNotes);
    setDefaultColours();
    
    setSpaceBetweenTiles (2);
    tree.addListener (this);
}


GridDisplayComponent::~GridDisplayComponent()
{

}


void GridDisplayComponent::paint (Graphics& g)
{
    g.setColour (Colours::black);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 5.0f);
}


Rectangle<int> GridDisplayComponent::getBoundsForTile (int column, int row)
{
    auto [x, y, w, h] = getRectangleDimentions (getLocalBounds());
    
    w /= numColumns;
    h /= numRows;

    return {
        x + column * w + halfSpaceBetweenTiles,
        y + row * h + halfSpaceBetweenTiles,
        w - spaceBetweenTiles,
        h - spaceBetweenTiles
    };
}


void GridDisplayComponent::resized()
{
    for (int column = 0; column < numColumns; ++column)
        for (int row = 0; row < numRows; ++row)
            tiles.getUnchecked (column)->getUnchecked (row)->setBounds (getBoundsForTile (column, row));
}

//===============================================================================================

void GridDisplayComponent::setSpaceBetweenTiles (int space) noexcept
{
    spaceBetweenTiles = space;
    halfSpaceBetweenTiles = space / 2;
    repaint();
}


void GridDisplayComponent::setStateForTile (int column, int row, TileState state) noexcept
{
    jassert (row < numRows && column < numColumns);
    
    auto tile = tree.getChildWithName (GridTileIdentifierManager::getIdentifierForIndex (column, row));
    
    tile.setProperty (IDs::Grid::TileState, VariantConverter<TileState>::toVar (state), nullptr);
}


void GridDisplayComponent::setSetabilityTile (int column, int row, bool setable) noexcept
{
    jassert (row < numRows && column < numColumns);
    
    auto tile = tree.getChildWithName (GridTileIdentifierManager::getIdentifierForIndex (column, row));
    
    tile.setProperty (IDs::Grid::TileSetable, setable, nullptr);
}


void GridDisplayComponent::setSetabilityColumn (int column, bool settable) noexcept
{
    jassert (column < numColumns);
    
    for (int row = 0; row < numRows; ++row)
        setSetabilityTile (column, row, settable);
}

//===============================================================================================

int GridDisplayComponent::getNumRows() const noexcept       { return numRows; }

int GridDisplayComponent::getNumColumns() const noexcept    { return numColumns; }


void GridDisplayComponent::turnAllTilesOff() noexcept
{
    for (int column = 0; column < numColumns; ++column)
        for (int row = 0; row < numRows; ++row)
            setStateForTile (column, row, TileState::tileInactive);
}


void GridDisplayComponent::valueTreePropertyChanged (ValueTree& tree, const Identifier& id)
{
    
    if (id == IDs::Grid::TileActiveColour || id == IDs::Grid::TileInactiveColour
        || id == IDs::Grid::TileRightColour || id == IDs::Grid::TileWrongColour)
        repaint();
    
    if (id == IDs::Grid::TileState)
        if (VariantConverter<TileState>::fromVar (tree[id]) == TileState::tileActive)
        {
            auto [column, row] = GridTileIdentifierManager::getIndexForIdentifier (tree.getType());
            setAllRowsInColumnInactiveExceptThisOne (column, row);
        }
}


void GridDisplayComponent::setAllRowsInColumnInactiveExceptThisOne (int column, int rowToLeaveActive)
{
    for (int row = 0; row < numRows; ++row)
        if (row != rowToLeaveActive)
            setStateForTile (column, row, TileState::tileInactive);
}


void GridDisplayComponent::setStateForTileInColumnWithThisRelativeNote (int column, int relativeNote, TileState state)
{
    for (int row = 0; row < numRows; ++row)
    {
        auto id = GridTileIdentifierManager::getIdentifierForIndex (column, row);
        auto tile = tree.getChildWithName (id);
        
        if (static_cast<int> (tile[IDs::Grid::TileRelativeNote]) == relativeNote)
            setStateForTile (column, row, state);
    }
}

int GridDisplayComponent::getRelativeNoteOfActiveTileInColumn (int column)
{
    for (int row = 0; row < numRows; ++row)
    {
        auto tileType = GridTileIdentifierManager::getIdentifierForIndex (column, row);
        auto tile = tree.getChildWithName (tileType);
        
        if (VariantConverter<TileState>::fromVar (tile[IDs::Grid::TileState]) == TileState::tileActive)
        {
            return tile[IDs::Grid::TileRelativeNote];
        }
    }
    
    return -1;
}


void GridDisplayComponent::initializeGridTiles (const StringArray& rowsText, const Array<int>& relativeNotes)
{
    for (int column = 0; column < numColumns; ++column)
    {
        OwnedArray<GridTileComponent>* col = new OwnedArray<GridTileComponent>();
        
        for (int row = 0; row < numRows; ++row)
        {
            auto type = GridTileIdentifierManager::getIdentifierForIndex (column, row);
            ValueTree childTile { type };
            tree.appendChild (childTile, nullptr);
            
            auto* tileptr = new GridTileComponent (childTile, type);
            
            col->set (row, tileptr);
            addAndMakeVisible (tileptr);
            
            childTile.setProperty (IDs::Grid::TileRelativeNote, relativeNotes[row], nullptr);
            childTile.setProperty (IDs::Grid::TileText,         rowsText[row],      nullptr);
        }
        
        tiles.set (column, col);
    }
}

void GridDisplayComponent::setDefaultColours()
{
    tree.setProperty (IDs::Grid::TileActiveColour,   VariantConverter<Colour>::toVar (Colours::gainsboro), nullptr);
    tree.setProperty (IDs::Grid::TileInactiveColour, VariantConverter<Colour>::toVar (Colours::black),     nullptr);
    tree.setProperty (IDs::Grid::TileRightColour,    VariantConverter<Colour>::toVar (Colours::green),     nullptr);
    tree.setProperty (IDs::Grid::TileWrongColour,    VariantConverter<Colour>::toVar (Colours::red),       nullptr);
}

//===============================================================================================
