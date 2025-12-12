/*
  ==============================================================================

    MelodyGenerator.h
    Created: 18 Oct 2019 10:03:48pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include "Identifiers.h"
#include "Utility.h"
#include <juce_gui_extra/juce_gui_extra.h>

//===============================================================================================
// This structure represents everything we need to know about a melody in this
// program

class Melody final : public juce::ReferenceCountedObject {
public:
  using Ptr = juce::ReferenceCountedObjectPtr<Melody>;

  Melody(juce::String mode, juce::Array<int> relativeNotes,
         int normalizedGround, int midiOffset, int noteLength,
         int timeBetweenNotes)
      : mode(mode), relativeNotes(relativeNotes),
        groundNoteIndex(normalizedGround), midiOffset(midiOffset),
        noteLength(noteLength), timeBetweenNotes(timeBetweenNotes) {}

  ~Melody() {}

  // this is used for answer checking
  static Ptr
  createMelodyWithOnlyRelativeNotesInfo(const juce::Array<int> &notes) {
    return new Melody{"X", notes, 0, 0, 0, 0};
  }

  juce::Array<int> getRelativeNotes() const { return relativeNotes; }

  juce::Array<int> generateMidiNotes() const {
    auto midiNotes = relativeNotes;

    for (auto &note : midiNotes)
      note += midiOffset;

    return midiNotes;
  }

  int getNumNotes() const { return relativeNotes.size(); }

  int getTimeBetweenNotes() const { return timeBetweenNotes; }

  int getNoteLength() const { return noteLength; }

  int getGroundNoteIndex() const { return groundNoteIndex; }

  int getRelativeGroundNote() const { return relativeNotes.getLast(); }

  int getRelativeFirstNote() const { return relativeNotes[0]; }

private:
  juce::String mode;
  juce::Array<int> relativeNotes;
  int groundNoteIndex;
  int midiOffset;
  int noteLength;
  int timeBetweenNotes;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Melody)
};

//===============================================================================================

template <> class juce::VariantConverter<Melody::Ptr> final {
public:
  static var toVar(const Melody::Ptr &melody) { return melody.get(); }

  static Melody::Ptr fromVar(const var &melody) {
    return dynamic_cast<Melody *>(melody.getObject());
  }
};

//===============================================================================================
// MelodyGenerator is the source for all training material in the program
//
// In MelodyGenerator there are 3 terms for note, which makes things a little
// confusing we have:
//      - Midi Notes that are the normal, known midi notes
//      - Normalized notes that are Midi Notes in terms of their distances, just
//      reduces to lower numbers
//        to make it easier to work with them to generate new midi
//      - Index notes, which are basically the index at which you can find a
//      note in the array of normalized notes,
//        these are neccesairy to make modulation possible without very complex
//        algorithms

class MelodyGenerator final {
public:
  MelodyGenerator(juce::ValueTree &t, int numNotes)
      : tree(t), numNotes(numNotes) {}

  Melody::Ptr generateMelody(int numNotes) noexcept {
    auto mode = getRandomMode();
    auto relativeNotes = generateRelativeNotesForMode(mode, numNotes);
    auto groundNoteIndex = getIndexGroundNoteForMode(mode);
    auto midiOffset = generateRandomMidiOffset();
    auto timeBetweenNotes = timeBetweenNotesMs;
    auto noteLength = noteLengthMs;

    return new Melody{mode,       relativeNotes, groundNoteIndex,
                      midiOffset, noteLength,    timeBetweenNotes};
  }

  void setNumNotesInMelody(int num) { numNotes = num; }

  void setTimeBetweenNotesMs(int timeInMs) { timeBetweenNotesMs = timeInMs; }

  void setNoteLengthInMs(int timeInMs) { noteLengthMs = timeInMs; }

  auto generateRandomMidiOffset() -> int { return random.nextInt(12) + 60; }

  void generateMelody() {
    Melody::Ptr melody = generateMelody(numNotes);
    tree.setProperty(IDs::Engine::EngineMelody, melody.get(), nullptr);
  }

  juce::String getRandomMode() noexcept {
    return modes[random.nextInt(modes.size())];
  }

  int getIndexGroundNoteForMode(const juce::String &mode) const noexcept {
    if (mode == "D")
      return 1;
    if (mode == "E")
      return 2;
    if (mode == "F")
      return 3;
    if (mode == "G")
      return 4;
    return 0;
  }

  juce::Array<int>
  generateMidiNotesFromRelativeNotes(juce::Array<int> relativeNotes) noexcept {
    auto transpose = random.nextInt(12) + 60;

    for (auto &note : relativeNotes)
      note += transpose;

    return relativeNotes;
  }

  juce::Array<int> generateRelativeNotesForMode(const juce::String &mode,
                                                int numNotes) noexcept {
    juce::Array<int> notes;

    juce::Array distances = {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4};

    auto groundIndex = getIndexGroundNoteForMode(mode);

    auto currentNoteIndex = groundIndex;

    notes.add(normalizedMidiNoteDistances[currentNoteIndex]);

    for (int i = 1; i < numNotes; ++i) {
      auto direction = random.nextBool() ? 1 : -1;

      auto interval = direction * distances[random.nextInt(distances.size())];

      currentNoteIndex = juce::jlimit(0, normalizedMidiNoteDistances.size() - 1,
                                      currentNoteIndex + interval);

      notes.add(normalizedMidiNoteDistances[currentNoteIndex]);
    }

    std::reverse(std::begin(notes), std::end(notes));

    return notes;
  }

  juce::Random random;
  juce::ValueTree tree;

  static inline const juce::Array<juce::String> modes{"D", "E", "F", "G"};
  int numNotes;
  static inline const juce::Array<int> normalizedMidiNoteDistances = {
      0, 2, 4, 5, 7, 9, 11};
  int timeBetweenNotesMs{400};
  int noteLengthMs{200};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelodyGenerator)
};
