

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "GridDisplayComponent.h"
#include "MelodyGenerator.h"
#include "TrainerEngine.h"
#include "ExtraMenus.h"
#include "AnswerChecker.h"

class MainComponent   : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent (juce::ValueTree& v);
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    //==============================================================================
    
    
    
private:
    //==============================================================================
    
    std::unique_ptr<ColourPickerWindow> colourPickerPanel;
    
    void initializeAudioSettings();
    
    juce::TextButton playButton       { "Start Playing"       };
    juce::TextButton generateButton   { "Generate Melody"     };
    juce::TextButton submitButton     { "Submit Answer"       };
    juce::TextButton infoButton       { "i"                   };
    //TextButton colourPickButton { "Open Colour Picker"  };
    
    juce::Label answerLabel ;
    
    juce::ValueTree tree;

    GridDisplayComponent gridDisplay;
    
    TrainerEngine trainerEngine;
    
    AnswerChecker answerChecker;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

