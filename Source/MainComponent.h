#pragma once

#include <JuceHeader.h>
#include <stdio.h>
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent()
        : state (stopped)
    {
        addAndMakeVisible(&openButton);
        openButton.setButtonText("Open...");
        openButton.onClick = [this] {openButtonClicked();};
        
        addAndMakeVisible(&playButton);
        playButton.setButtonText("Play");
        playButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.onClick = [this] {playButtonClicked();};
        playButton.setEnabled(false);
        
        addAndMakeVisible(&stopButton);
        stopButton.setButtonText("Stop");
        stopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
        stopButton.onClick = [this] {stopButtonClicked();};
        stopButton.setEnabled(false);
        
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
        
        
        setSize (800, 600);
        
        setAudioChannels (0, 2);
    }

    ~MainComponent()
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr){
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        transportSource.getNextAudioBlock(bufferToFill);
        
    }
    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
            if(source == &transportSource){
                if(transportSource.isPlaying()){
                    changeState(playing);
                } else {
                    changeState(stopped);
                }
            }
    }
    //==============================================================================
    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
        playButton.setBounds (10, 40, getWidth() - 20, 20);
        stopButton.setBounds (10, 70, getWidth() - 20, 20);
    }
private:
    //==============================================================================
    // Your private member variables go here...
    enum TransportState{
        stopped,
        starting,
        playing,
        stopping
    };
    void openButtonClicked()
    {
        juce::FileChooser fileChooser ("Choose a file to play. . . ", {}, "*.wav"); //creates a FileChooser object
        if(fileChooser.browseForFileToOpen()){ //this 'if' evaluates true if the user chooses a file rather than clicking cancel
            auto chosenFile = fileChooser.getResult();
            auto reader = formatManager.createReaderFor(chosenFile); //creates a reader object specific to the chosen file
            if(reader != nullptr){ //the reader returns a nullptr if the chosen file is not a valid audio format
                //below creates an AudioFormatReaderSource object from the AudioFormatReader (reader) variable just created
                //the 'true' arg indicates that newSource will automatically manage memory for the initial reader obj and delete it as necessary
                std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
                //below send the audio data from newSource to the transportSource object used in the getNextAudioBlock() func
                transportSource.setSource(newSource.get(),0, nullptr, reader->sampleRate);
                playButton.setEnabled(true);
                readerSource.reset(newSource.release()); //releases the unique_ptr for newSource bc processing has been handed off to the transportSource
            }
        }
        
    }
    void playButtonClicked()
    {
        changeState(starting);
    }
    void stopButtonClicked()
    {
        changeState(stopping);
    }

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    TransportState state;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    void changeState(TransportState newState){
        if (state != newState){
            state = newState;
            switch(state){
                case stopped:
                    stopButton.setEnabled(false);
                    playButton.setEnabled(true);
                    transportSource.setPosition(0.0);
                    break;
                case starting:
                    playButton.setEnabled(false);
                    transportSource.start();
                    break;
                case playing:
                    stopButton.setEnabled(true);
                    break;
                case stopping:
                    transportSource.stop();
                    break;
            }
        }
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
// end of Main Component
