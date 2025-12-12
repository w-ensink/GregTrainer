/*
  ==============================================================================

    Synth.h
    Created: 15 Oct 2019 9:32:07pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include "Utility.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
// For now this is a simple sine synth from a Juce example
// as I haven't had the time to make a synth myself yet.

struct SineWaveSound : public juce::SynthesiserSound {
  SineWaveSound() {}

  bool appliesToNote(int) override { return true; }
  bool appliesToChannel(int) override { return true; }
};

//==============================================================================

struct SineWaveVoice : public juce::SynthesiserVoice {
  SineWaveVoice() {}

  bool canPlaySound(juce::SynthesiserSound *sound) override {
    return dynamic_cast<SineWaveSound *>(sound) != nullptr;
  }

  void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound *,
                 int currentPitchWheelPosition) override {
    currentAngle = 0.0;
    level = velocity * 0.15;
    tailOff = 0.0;

    auto cyclesPerSecond =
        juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();

    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
  }

  void stopNote(float velocity, bool allowTailOff) override {
    if (allowTailOff) {
      if (tailOff == 0.0)
        tailOff = 1.0;
    } else {
      clearCurrentNote();
      angleDelta = 0.0;
    }
  }

  void pitchWheelMoved(int) override {}
  void controllerMoved(int, int) override {}

  void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample,
                       int numSamples) override {
    if (angleDelta != 0.0) {
      if (tailOff > 0.0) {
        while (--numSamples >= 0) {
          auto currentSample =
              (float)(std::sin(currentAngle) * level * tailOff);

          for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

          currentAngle += angleDelta;
          ++startSample;

          tailOff *= 0.99;

          if (tailOff <= 0.005) {
            clearCurrentNote();

            angleDelta = 0.0;
            break;
          }
        }
      } else {
        while (--numSamples >= 0) {
          auto currentSample = (float)(std::sin(currentAngle) * level);

          for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

          currentAngle += angleDelta;
          ++startSample;
        }
      }
    }
  }

private:
  double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
};

//==============================================================================
// To make use of more generic audio processors possible in the engine

class InternalProcessorBase : public juce::AudioProcessor {
public:
  juce::AudioProcessorEditor *createEditor() override { return nullptr; }
  bool hasEditor() const override { return false; }
  int getNumPrograms() override { return 0; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override {
    return "InternalProcessor";
  }
  void changeProgramName(int, const juce::String &) override {}
  void getStateInformation(juce::MemoryBlock &) override {}
  void setStateInformation(const void *, int) override {}

  double getTailLengthSeconds() const override { return 0.0; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }

  const juce::String getName() const override { return "InternalProcessor"; }

  void prepareToPlay(double, int) override {}
  void releaseResources() override {}
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override {}
};

//==============================================================================
// Basic implementation of the simple sine synth that is used by default

class SineWaveSynthesizer : public InternalProcessorBase {
public:
  SineWaveSynthesizer() {
    synth.addVoice(new SineWaveVoice());
    synth.addSound(new SineWaveSound());
  }

  void prepareToPlay(double sampleRate,
                     int maximumExpectedSamplesPerBlock) override {
    synth.setCurrentPlaybackSampleRate(sampleRate);
  }

  void releaseResources() override {}

  void processBlock(juce::AudioBuffer<float> &buffer,
                    juce::MidiBuffer &midiMessages) override {
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
  }

private:
  juce::Synthesiser synth;
};
