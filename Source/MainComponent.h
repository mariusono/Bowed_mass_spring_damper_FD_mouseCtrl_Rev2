#pragma once

#include <JuceHeader.h>
#include "mass_spring_damper.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, 
					   public juce::HighResolutionTimer, // for communication with phantom 
	                   public juce::Timer, // for graphics refresh
					   public juce::Slider::Listener,
					   public juce::MouseListener
{
public:
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
    
    // Mouse functions
    void mouseDown (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;
	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& w) override;

	// Hi resolution timer callback
	void hiResTimerCallback() override;

	// Graphics timer callback
	void timerCallback() override;

	int idx_mass = 0;
	double wheelDeltaY = 0.0;

	// Slider stuff
	void sliderValueChanged(Slider* slider) override;


private:
    //==============================================================================
    double fs;
    double bufferSize;
    float minOut;
    float maxOut;
    
	double globalCurrentSample;

	OwnedArray<mass_spring_damper> mass_spring_dampers;
    std::vector<mass_spring_damper*> activeMasses; // why the * ? 

	//std::vector<std::unique_ptr<mass_spring_damper>> mass_spring_dampers;
	//std::vector<std::unique_ptr<mass_spring_damper>> activeMasses;

    int numMasses;
    int octave;
    double FB_Max;
    //std::vector<const char> keys = {'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G', 'Y', 'H', 'U', 'J', 'K'}; // const char does not work in this version of VS ... !!!
	std::vector<char> keys = { 'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G', 'Y', 'H', 'U', 'J', 'K' };

    int polyphony;
    int currentPoly = 0;

	std::vector<double> preOutputVec;
	std::vector<bool> keyDownVec;

    
    double eYPrev = 0;
    long int timePrev = 0;;

    std::vector<double> gPosition;
    std::vector<double> gPositionRaw;
	std::vector<double> gPositionPrev;

	// SLIDER STUFF
	Slider dampingSlider;
	Label dampingLabel;

    Slider frParamSlider;
    Label frParamLabel;
    
    Slider volumeSlider;
    Label volumeLabel;
    
	double gDampingVal;
    double gFrParam;
    double gVolume;

    // For graphics
    // Opacity
    float opa_level = 1.0;
    bool flagMouseUp = true;

    // FOR DC BLOCKER
    double y_nm1;
    double y_n;
    double x_nm1;
    double x_n;
    double R_fac;
    
    // FOR LIMITER
    
    double at;
    double rt;
    int delaySamples;
    double outPeak;
    double g_lim;
    double coeff;
    double lt;
    double outputLim;
    int BUFFER_SIZE = 1024;
    
    int gReadPointer_limiter;
    int gWritePointer_limiter;
    std::vector<double> delayLine_limiter;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
