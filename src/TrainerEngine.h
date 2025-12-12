/*
  ==============================================================================

    TrainerEngine.h
    Created: 25 Oct 2019 3:32:16pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "MelodyGenerator.h"
#include "MidiGenerator.h"

//=======================================================================
// This is the main engine for the trainer
// it provides everything we need that is not the interface

class TrainerEngine final : public juce::AudioSource,
                            private juce::ValueTree::Listener,
                            private juce::AsyncUpdater {
public:
  enum class PlayState { playing, checkingAnswer, stopped };

  TrainerEngine(juce::ValueTree &, int numNotes);

  ~TrainerEngine() override;

  //===================================================================

  void prepareToPlay(int, double) override;

  void getNextAudioBlock(const juce::AudioSourceChannelInfo &) override;

  void releaseResources() override;

  //===================================================================

  void setNumNotesInMelody(int);

  void setTimeBetweenNotesInMs(int);

  void setNoteLengthInMs(int);

  void generateNextMelody();

  // this should put the trainer in checking state until a new
  // melody is generated (to be implemented)
  void checkAnswer();

  void startPlayingMelody();

  void stopPlayingMelody();

  void checkIfMelodyIsSameAsPlayed(Melody &);

  //===================================================================

  void valueTreePropertyChanged(juce::ValueTree &,
                                const juce::Identifier &) override;

  //===================================================================

  bool openInstrumentEditor();

  //===================================================================

  void handleAsyncUpdate() override;

private:
  juce::ValueTree engineState;

  juce::CachedValue<PlayState> playState;

  std::unique_ptr<juce::AudioProcessor> playbackInstrument;

  double currentSampleRate;

  MelodyGenerator melodyGenerator;
  MidiGenerator midiGenerator;
  juce::CachedValue<bool> isPlaying;

  //===================================================================

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrainerEngine)
};

template <> class juce::VariantConverter<TrainerEngine::PlayState> final {
public:
  using State = TrainerEngine::PlayState;

  static var toVar(const State &state) {
    if (state == State::playing)
      return "p";
    if (state == State::checkingAnswer)
      return "c";
    if (state == State::stopped)
      return "s";

    return "undefined";
  }

  static State fromVar(const var &state) {
    if (state == "p")
      return State::playing;
    if (state == "c")
      return State::checkingAnswer;
    if (state == "s")
      return State::stopped;

    return State::stopped;
  }
};
