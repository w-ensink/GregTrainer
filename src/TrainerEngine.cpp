/*
  ==============================================================================

    TrainerEngine.cpp
    Created: 25 Oct 2019 3:32:16pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#include "TrainerEngine.h"
#include "Synth.h"
#include "Identifiers.h"

TrainerEngine::TrainerEngine (ValueTree& tree, int numNotes) :
        engineState {IDs::Engine::EngineRoot}, melodyGenerator (engineState, numNotes)
{
    
    engineState.addListener (this);
    tree.appendChild (engineState, nullptr);
    
    playState.referTo (engineState, IDs::Engine::PlayState, nullptr, PlayState::stopped);
    
    playbackInstrument.reset (new SineWaveSynthesizer());
}

TrainerEngine::~TrainerEngine()
{
    
}

//==================================================================================


void TrainerEngine::prepareToPlay (int numSamplesPerBlockExpected, double sampleRate)
{
    if (playbackInstrument != nullptr)
        playbackInstrument->prepareToPlay (sampleRate, numSamplesPerBlockExpected);
    
    midiGenerator.setSampleRate (sampleRate);
    currentSampleRate = sampleRate;
}


void TrainerEngine::getNextAudioBlock (const AudioSourceChannelInfo& channelInfo)
{
    auto midiBuffer = MidiBuffer();
    auto numSamples = channelInfo.buffer->getNumSamples();

    midiGenerator.renderNextMidiBlock (midiBuffer, numSamples);
    
    if (playbackInstrument != nullptr)
        playbackInstrument->processBlock (*channelInfo.buffer, midiBuffer);
    
}


void TrainerEngine::releaseResources()
{
    if (playbackInstrument != nullptr)
        playbackInstrument->releaseResources();
}


//==================================================================================


void TrainerEngine::setNumNotesInMelody (int numNotes)
{
    melodyGenerator.setNumNotesInMelody (numNotes);
}

void TrainerEngine::setTimeBetweenNotesInMs (int intervalTimeMs)
{
    melodyGenerator.setTimeBetweenNotesMs (intervalTimeMs);
}

void TrainerEngine::setNoteLengthInMs (int timeInMs)
{
    melodyGenerator.setNoteLengthInMs (timeInMs);
}

void TrainerEngine::generateNextMelody()
{
    melodyGenerator.generateMelody();
    
    auto melody = VariantConverter<Melody::Ptr>::fromVar (engineState[IDs::Engine::EngineMelody]);

    midiGenerator.setMelody (melody);
}

void TrainerEngine::startPlayingMelody()
{
    midiGenerator.startPlaying();
}

void TrainerEngine::stopPlayingMelody()
{
    midiGenerator.stopPlaying();
}

void TrainerEngine::checkIfMelodyIsSameAsPlayed (Melody&)
{
    
}


//==================================================================================

void TrainerEngine::valueTreePropertyChanged (ValueTree& t, const Identifier& id)
{
    if (id == IDs::Engine::PlayState)
        triggerAsyncUpdate();
}

//==================================================================================

bool TrainerEngine::openInstrumentEditor()
{
    return playbackInstrument->hasEditor();
}

// handleAsyncUpdate handles the state changes
void TrainerEngine::handleAsyncUpdate()
{
    if (playState == PlayState::playing)
        startPlayingMelody();
    
    if (playState == PlayState::stopped)
        stopPlayingMelody();
}
