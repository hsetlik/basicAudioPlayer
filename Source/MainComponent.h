#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    enum TransportState{
        stopped,
        starting,
        playing,
        stopping
    };
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
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

private:
    //==============================================================================
    // Your private member variables go here...
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    TransportState state;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
