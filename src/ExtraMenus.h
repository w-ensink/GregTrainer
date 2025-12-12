/*
  ==============================================================================

    ExtraMenus.h
    Created: 25 Oct 2019 6:09:08pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "Identifiers.h"

//==============================================================================
// basic info panel with some info about author and where requests can be sent

class InfoPanelComponent : public juce::Component {
public:
  InfoPanelComponent() {
    addAndMakeVisible(emailLink);
    juce::Font fond{"Arial", 25.0f, juce::Font::plain};
    emailLink.setFont(fond, false);
    emailLink.setColour(juce::HyperlinkButton::textColourId,
                        juce::Colours::blue);
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(30.0f);

    g.drawText("Author: Wouter Ensink",
               getLocalBounds().toFloat().withTrimmedBottom(getHeight() / 2),
               juce::Justification::centred);

    g.setFont(25.0f);

    const auto bounds = getLocalBounds().toFloat();

    const auto textBounds = bounds.withTrimmedTop(getHeight() / 3)
                                .withTrimmedBottom(getHeight() / 3);

    g.drawText(
        "Send Bugs or Requests to:", textBounds.translated(0, getHeight() / 6),
        juce::Justification::horizontallyCentred);
  }

  void resized() override {
    auto bounds = getLocalBounds()
                      .withTrimmedTop(getHeight() / 3)
                      .withTrimmedBottom(getHeight() / 3);
    emailLink.setBounds(bounds.translated(0, getHeight() / 3));
  }

  juce::HyperlinkButton emailLink{"wouter.ensink@student.hku.nl",
                                  {"wouter.ensink@student.hku.nl"}};
};

//===============================================================================================
// Colour Picker to set the colour of the grid

class ColourPickerWindow : public juce::DocumentWindow,
                           public juce::ChangeListener {
public:
  // needs an onClosing lambda to set the pointer holding this to null..

  ColourPickerWindow(juce::ValueTree &t, std::function<void()> onClosing)
      : DocumentWindow("Colour Picker", juce::Colours::black,
                       TitleBarButtons::closeButton),
        tree(t), onClosing(onClosing) {
    setSize(100, 100);
    setContentOwned(selector = new juce::ColourSelector(), false);

    selector->addChangeListener(this);

    setResizable(true, false);
    setResizeLimits(300, 400, 800, 1500);
    setTopLeftPosition(60, 60);

    setVisible(true);
  }

  void closeButtonPressed() override { onClosing(); }

private:
  void changeListenerCallback(juce::ChangeBroadcaster *broadcaster) override {
    auto gridTree = tree.getChildWithName(IDs::Grid::GridRoot);

    if (broadcaster == selector)
      gridTree.setProperty(IDs::Grid::TileInactiveColour,
                           selector->getCurrentColour().toString(), nullptr);
  }

  juce::ColourSelector *selector;
  juce::ValueTree tree;
  std::function<void()> onClosing;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourPickerWindow)
};
