/*
  ==============================================================================

    MidiGenerator.h
    Created: 15 Oct 2019 9:32:25pm
    Author:  Wouter Ensink

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Utility.h"


// MidiGenerator is the piece of code that translates the information from a Melody object into
// actual MIDI and fills buffers with that MIDI once startPlaying() is called

class MidiGenerator final
{
public:
    
    MidiGenerator() { }
    ~MidiGenerator() { }
    
    void setSpeed() { }
    
    void setSampleRate (double newSampleRate) noexcept
    {
        sampleRate = newSampleRate;
        recalculateSettings();
    }
    
    void setMelody (Melody::Ptr melody) noexcept
    {
        if (melody != nullptr)
        {
            notesToPlay = melody->generateMidiNotes();
            setTimeBetweenNotesInMs (melody->getTimeBetweenNotes());
            setNoteLengthInMs (melody->getNoteLength());
        }
        else
        {
            print ("error MidiGenerator::setMelody: melody == nullptr");
        }
    }
    
    void startPlaying() noexcept
    {
        notesIndexNoteOn = 0;
        notesIndexNoteOff = 0;
        remainderNoteOn = numSamplesBetweenNotes;
        remainderNoteOff = remainderNoteOn + noteLengthInSamples;
        isCurrentlyPlaying = true;
    }
    
    void stopPlaying() noexcept
    {
        isCurrentlyPlaying = false;
    }
    
    // fills the midibuffer with messages if needed
    void renderNextMidiBlock (MidiBuffer& buffer, int numSamples) noexcept
    {
        if (isCurrentlyPlaying)
        {
            if (remainderNoteOn < numSamples)
            {
                if (auto note = getNextMidiNoteOn(); note != 0)
                {
                    buffer.addEvent (MidiMessage::noteOn (1, note, 0.9f), remainderNoteOn);
                    remainderNoteOn = numSamplesBetweenNotes - (numSamples - remainderNoteOn);
                }
            }
            else
            {
                remainderNoteOn -= numSamples;
            }
            
            
            if (remainderNoteOff < numSamples)
            {
                if (auto note = getNextMidiNoteOff(); note != 0)
                {
                    buffer.addEvent (MidiMessage::noteOff (1, note, 0.0f), remainderNoteOff);
                    remainderNoteOff = numSamplesBetweenNotes - (numSamples - remainderNoteOff);
                }
            }
            else
            {
                remainderNoteOff -= numSamples;
            }
        }
    }
    
    
    
private:
    
    void setTimeBetweenNotesInMs (int timeInMs) noexcept
    {
        timeBetweenNotesInMs = timeInMs;
        recalculateSettings();
    }

    void setNoteLengthInMs (int timeInMs) noexcept
    {
        noteLengthInMs = timeInMs;
        recalculateSettings();
    }
    

    int getNextMidiNoteOn() noexcept
    {
        return notesIndexNoteOn < notesToPlay.size() ? notesToPlay[notesIndexNoteOn++] : 0;
    }
    
    int getNextMidiNoteOff() noexcept
    {
        return notesIndexNoteOff < notesToPlay.size() ? notesToPlay[notesIndexNoteOff++] : 0;
    }
    
    void recalculateSettings()
    {
        noteLengthInSamples = noteLengthInMs * sampleRate * 0.001;
        numSamplesBetweenNotes = timeBetweenNotesInMs * sampleRate * 0.001;
    }
    
    
    int numSamplesBetweenNotes;
    int remainderNoteOn;
    int remainderNoteOff;
    Array<int> notesToPlay;
    int notesIndexNoteOn;
    int notesIndexNoteOff;
    double sampleRate;
    int timeBetweenNotesInMs;
    int noteLengthInMs;
    int noteLengthInSamples;
    bool isCurrentlyPlaying { false };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiGenerator)
    
};


