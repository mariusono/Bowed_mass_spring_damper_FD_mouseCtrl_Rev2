#include "MainComponent.h"

// OTHER FUNCTIONS

double linearMapping(float rangeIn_top, float rangeIn_bottom, float rangeOut_top, float rangeOut_bottom, float value) {
	double newValue = rangeOut_bottom + ((rangeOut_top - rangeOut_bottom) * (value - rangeIn_bottom) / (rangeIn_top - rangeIn_bottom));
	return newValue;
}


double exponentialMapping(float rangeIn_top, float rangeIn_bottom, float rangeOut_top, float rangeOut_bottom, float fac, float value)
{
    // make sure values passed to function are within the rangeIn_bottom rangeIn_top interval !!
    // maybe add an error exception here..
    // first map value to rangeIn to 0 - 1
    double valueMapped = 0.0 + ((1.0 - 0.0) * (value - rangeIn_bottom) / (rangeIn_top - rangeIn_bottom));
    
    // map to an exponential curve between 0 and 1 with a factor fac
    double mapToExp = (exp(valueMapped * fac) - 1) / (exp(fac)-1);

    // map back to desired output range
    double newValue = rangeOut_bottom + ((rangeOut_top - rangeOut_bottom) * (mapToExp - 0.0) / (1.0 - 0.0));

    return newValue;
}

std::vector<float> gOutputMasses;


//==============================================================================
MainComponent::MainComponent() : minOut(-1.0), maxOut(1.0), numMasses(11), octave(0), polyphony(12)
{
	// Make sure you set the size of the component after
	// you add any child components.
	setSize(800, 600);

	// specify the number of input and output channels that we want to open
	setAudioChannels(0, 2);

    setMouseCursor(MouseCursor::NoCursor); // remove mouse cursor from this component
	//setWantsKeyboardFocus(true);
	//addKeyListener(this);

	// SLIDER STUFF:

	addAndMakeVisible(dampingSlider);
	dampingSlider.setSliderStyle(juce::Slider::Rotary);
	dampingSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); // Add a textbox..
	dampingSlider.setTextValueSuffix(" [-]");    
	dampingSlider.setRange(0.01, 100.0);          // maybe scale these values afterwards
	dampingSlider.setValue(15.0); // with exp mapping this will result in 10.0ish
	dampingSlider.addListener(this);             

	dampingLabel.setText("Sound Decay", juce::dontSendNotification);
	dampingLabel.attachToComponent(&dampingSlider, true); // [4]
	addAndMakeVisible(dampingLabel);
    
    
    addAndMakeVisible(frParamSlider);
    frParamSlider.setSliderStyle(juce::Slider::Rotary);
    frParamSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    frParamSlider.setTextValueSuffix(" [-]");
//    frParamSlider.setRange(1.0, 400.0); // maybe scale these values afterwards. i.e. exp mapping
//    frParamSlider.setRange(1.0, 1500.0);
    frParamSlider.setRange(1.0, 15000.0);
//    frParamSlider.setValue(409.0); // with exp mapping this will result in 100.0ish
    frParamSlider.setValue(7500.0); // with exp mapping this will result in 100.0ish
    frParamSlider.addListener(this);

    frParamLabel.setText("Friction Amount", juce::dontSendNotification);
    frParamLabel.attachToComponent(&frParamSlider, true); // [4]
    addAndMakeVisible(frParamLabel);
    
    
    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::Rotary);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setTextValueSuffix(" [-]");
    volumeSlider.setRange(-10.0, 3.0); // in dB. conversion done in sliderValueChanged routine
    volumeSlider.setValue(-1.0);
    volumeSlider.addListener(this);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent(&volumeSlider, true);
    addAndMakeVisible(volumeLabel);

}

MainComponent::~MainComponent()
{
	// This shuts down the audio device and clears the audio source.
	shutdownAudio();

	//// PHANTOM STUFF
	//hdStopScheduler();
	//hdDisableDevice(hHD);

	//// Stop the graphics update
	//stopTimer();
}

void MainComponent::sliderValueChanged(Slider* slider)
{
	if (slider == &dampingSlider)
	{
		gDampingVal = dampingSlider.getValue();
        gDampingVal = exponentialMapping(100.0, 0.01, 100.0, 0.01, 3, gDampingVal);
        
		Logger::getCurrentLogger()->outputDebugString("gDampingVal: (" + String(gDampingVal) + ")");
	}
    else if (slider == &frParamSlider)
    {
        gFrParam = frParamSlider.getValue();
////        gFrParam = exponentialMapping(1500.0, 1.0, 1500.0, 1.0, 4.0, gFrParam);
//        gFrParam = exponentialMapping(15000.0, 1.0, 15000.0, 1.0, 8.0, gFrParam);
        gStickFact = linearMapping(15000.0, 1.0, 2.0, 0.0, gFrParam);
        gFrParam = linearMapping(15000.0, 1.0, 1.0, 15000.0, gFrParam);
        gFrParam = exponentialMapping(1.0, 15000.0, 1.0, 15000.0, -8.0, gFrParam);
//        gStickFact = linearMapping(1.0, 15000.0, 2.0, 0.0, gFrParam);

//        Logger::getCurrentLogger()->outputDebugString("gFrParam: (" + String(gFrParam) + ")");
        Logger::getCurrentLogger()->outputDebugString("gStickFact: (" + String(gStickFact) + ")");
    }
    else if (slider == &volumeSlider)
    {
        gVolume = volumeSlider.getValue();
        gVolume = powf(10.0, gVolume / 20);
        if (gVolume < 0.317)
        {
            gVolume = 0;
        }
        
        Logger::getCurrentLogger()->outputDebugString("gVolume: (" + String(gVolume) + ")");

    }
}


//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	// This function will be called when the audio device is started, or when
	// its settings (i.e. sample rate, block size, etc) are changed.

	// You can use this function to initialise any resources you might need,
	// but be careful - it will be called on the audio thread, not the GUI thread.

	// For more details, see the help for AudioProcessor::prepareToPlay()

	globalCurrentSample = 0;

	fs = sampleRate;
	bufferSize = samplesPerBlockExpected;

	int test = 0; // why use this..
	for (int i = test; i < numMasses + test; ++i)
	{
		//mass_spring_dampers.add(new mass_spring_damper(110 * pow(2, i / 12.0) + 1, fs));
		mass_spring_dampers.add(new mass_spring_damper(55 + 27.5 * i, fs));
//		mass_spring_dampers.add(new mass_spring_damper(220 + 27.5 * i, fs));
		//mass_spring_dampers.add(new mass_spring_damper(110 + 110 * i, fs));
		//mass_spring_dampers.add(new mass_spring_damper (55 * pow (2, i / 12.0), fs));
        
        mass_spring_dampers[i]->setFb(100.0);
	}

    FB_Max = 1000.0;
//    opa_level = 1000.0/FB_Max;
    opa_level = 0.1;
    
	//int test = 0;
	//for (int i = test; i < numMasses + test; ++i)
	//{
	//	mass_spring_dampers.push_back(std::make_unique<mass_spring_damper>(55.0 * pow(2, i / 12.0), fs));
	//}

	preOutputVec.resize(numMasses, 0);
	gOutputMasses.resize(numMasses, 0);
	keyDownVec.resize(numMasses, false);

	int a = 0;
	activeMasses.resize(polyphony, nullptr);

	// Timer for graphics
	//startTimerHz(15); // start the timer (15 Hz is a nice tradeoff between CPU usage and update speed)
	startTimerHz(24); // good ol 24 fps

	// Timer Callback for parameter variation
	HighResolutionTimer::startTimer(1000.0 / 150.0); // 150 Hz
	//HighResolutionTimer::startTimer(1000.0 / 1.0); // 1 Hz


    gPosition.resize(2, 0);
    gPositionRaw.resize(2, 0);
	gPositionPrev.resize(2, 0);
    
    
    // LIMITER STUFF:
    delayLine_limiter.resize(BUFFER_SIZE, 0);
    delaySamples = 100;
    outPeak = 0; // initialize peak out
    at = 0.3;
    rt = 0.01;
    g_lim = 0.0;
//    lt = 0.4; // some reasonable value..
    lt = 0.6; // some reasonable value..

    gReadPointer_limiter = 0;
    gWritePointer_limiter = 0;
    
    // DC BLOCKER STUFF
    
    y_nm1 = 0.0;
    y_n = 0.0;
    x_nm1 = 0.0;
    x_n = 0.0;
    R_fac = 0.995;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	// updating global model parameters every buffersize.. for efficiency. may cause some clicks
	for (int iMass = 0; iMass < numMasses; ++iMass)
	{
        mass_spring_dampers[iMass]->setDamping(gDampingVal);
        mass_spring_dampers[iMass]->setFrParam(gFrParam);
        mass_spring_dampers[iMass]->setStickFact(gStickFact);
	}

    // update volume at every buffersize to avoid clicks.. (maybe)
    double volPerBuffer = gVolume;
    
	// Your audio-processing code goes here!
	for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
	{
		float *const channelData = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

		if (channel == 0)
		{
			for (int i = 0; i < bufferToFill.buffer->getNumSamples(); i++)
			{

				float output = 0.0;
				for (int j = 0; j < numMasses; ++j)
				{
//					if (mass_spring_dampers[j]->isActive())
//					{
//						float massSound = mass_spring_dampers[j]->process() * 600; // this is some global amplitude here
                        
//                        double exponentialMapping(float rangeIn_top, float rangeIn_bottom, float rangeOut_top, float rangeOut_bottom, float fac, float value)
                        
                        float gainVal = exponentialMapping((float) numMasses - 1.0, 0.0, 550.0, 120.0, -1.5, (float) j); // this is a reasonable mapping..
//                        gainVal = 500;
                        float massSound = mass_spring_dampers[j]->process() * gainVal; // this is some global amplitude here
                        
//                        if (i % bufferToFill.buffer->getNumSamples() < 1 && mass_spring_dampers[j]->isActive())
//                        {
//                            Logger::getCurrentLogger()->outputDebugString("rawOut: (" + String(mass_spring_dampers[j]->process()) + ")");
//                        }
                        
						gOutputMasses[j] = massSound;
						output = output + massSound;
//					}
				}
                output = output * volPerBuffer; // add global volume contribution
                
                // ADD DC BLOCKER
                x_n = output;
                y_n = x_n - x_nm1 + R_fac * y_nm1;
                
                x_nm1 = x_n;
                y_nm1 = y_n;
                
                output = y_n;
                
                // ADD LIMITER
                double a = abs(output);
                if (a>outPeak)
                {
                    coeff = at;
                }
                else
                {
                    coeff = rt;
                }
                outPeak = (1-coeff) * outPeak + coeff * a;
                double f = juce::jmin((double) 1.0, lt);
                if (f<g_lim)
                {
                    coeff = at;
                }
                else
                {
                    coeff = rt;
                }
                g_lim = (1-coeff) * g_lim + coeff * f;
                
                gReadPointer_limiter = (gWritePointer_limiter - delaySamples + BUFFER_SIZE) % BUFFER_SIZE;

                outputLim = g_lim * delayLine_limiter.at(gReadPointer_limiter);
//                outputLim = output;

                delayLine_limiter.at(gWritePointer_limiter) = output;
                
                gWritePointer_limiter = gWritePointer_limiter + 1;
                if (gWritePointer_limiter >= BUFFER_SIZE)
                {
                    gWritePointer_limiter = 0;
                }
                
                
				if (outputLim > maxOut) // output is limited..
				{
                    Logger::getCurrentLogger()->outputDebugString("Output is too large!");
                    outputLim = maxOut;
				}
				else if (outputLim < minOut) {
                    Logger::getCurrentLogger()->outputDebugString("Output is too small!");
                    outputLim = minOut;
				}
				channelData[i] = outputLim;
				//gOutput = output;
			}
		}
		else
		{
			memcpy(channelData,
				bufferToFill.buffer->getReadPointer(0),
				bufferToFill.numSamples * sizeof(double));
		}
	}
}

void MainComponent::releaseResources()
{
	// This will be called when the audio device stops, or when it is being
	// restarted due to a setting change.

	// For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	// You can add your drawing code here!
    
    Rectangle<int> area = getLocalBounds();

	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    g.setOpacity(1.0);
	for (int i = 0; i < numMasses; ++i)
	{
		g.setColour(Colour::fromRGB(50 + i * 200.0 / static_cast<double> (numMasses), 0, 0));

        g.fillEllipse(area.getWidth()/2-area.getWidth() / static_cast<double> (numMasses)/2 + gOutputMasses[i] * 100, round(i * getHeight() / static_cast<double> (numMasses)), area.getWidth() / static_cast<double> (numMasses), area.getHeight() / static_cast<double> (numMasses));
        
//		g.fillEllipse(getWidth() / 2.0 + gOutputMasses[i] * 100, round(i * getHeight() / static_cast<double> (numMasses)),
//					  getWidth() / static_cast<double> (numMasses),
//					  getHeight() / static_cast<double> (numMasses));

	}

    if (flagMouseUp)
    {
        g.setColour(Colours::grey);
        g.setOpacity(opa_level);
    }
    else
    {
        g.setColour(Colours::orange);
        g.setOpacity(opa_level);
    }

//    g.fillEllipse(gPositionRaw[0]-30/2,gPositionRaw[1]-30/2,30,30);
    
    int xRect = gPositionRaw[0] - (area.getWidth()/2)/2;
    int yRect = gPositionRaw[1] - 30/2;
    int widthRect = area.getWidth()/2;
    int heightRect = 30;
    
//    int xRect = gPositionRaw[0] - (area.getX()/3)/2;
//    int yRect = gPositionRaw[1];
//    int widthRect = 30;
//    int heightRect = 30;
    
    g.fillRect(xRect,yRect,widthRect,heightRect);

}

void MainComponent::resized()
{
	// This is called when the MainContentComponent is resized.
	// If you add any child components, this is where you should
	// update their positions.

//	auto border = 4;
	Rectangle<int> area = getLocalBounds();
    
    int x_dampSlider = area.getX() + area.getWidth() / 8;
    int y_dampSlider = area.getY() + area.getHeight() / 8;
    int width_dampSlider = 100;
    int height_dampSlider = 100;

    dampingSlider.setBounds(x_dampSlider,y_dampSlider,width_dampSlider,height_dampSlider);
    
    int x_frParamSlider = area.getX() + area.getWidth() / 8;
    int y_frParamSlider = 120 + area.getY() + area.getHeight() / 8;
    int width_frParamSlider = 100;
    int height_frParamSlider = 100;
    
    frParamSlider.setBounds(x_frParamSlider, y_frParamSlider, width_frParamSlider, height_frParamSlider);
    
    volumeSlider.setBounds(x_dampSlider,y_dampSlider,width_dampSlider,height_dampSlider);
    
    int x_volumeSlider = area.getX() + area.getWidth() / 8;
    int y_volumeSlider = 240 + area.getY() + area.getHeight() / 8;
    int width_volumeSlider = 100;
    int height_volumeSlider = 100;
    
    volumeSlider.setBounds(x_volumeSlider, y_volumeSlider, width_volumeSlider, height_volumeSlider);
}

void MainComponent::timerCallback()
{
	repaint(); // update the graphics X times a second

	const ComponentPeer* const peer = getPeer();
    
	auto aba = peer->getNativeHandle();
	int ana = 3;
}



void MainComponent::hiResTimerCallback()
{
//    repaint(); // update the graphics X times a second

}

void MainComponent::mouseMove(const MouseEvent &e)
{
    gPositionRaw[0] = (double) e.x;
    gPositionRaw[1] = (double) e.y;
}


void MainComponent::mouseDown(const MouseEvent &e)
{
//    Logger::getCurrentLogger()->outputDebugString("Mouse down!");

	//float scaledEX = e.x / static_cast<double> (getWidth());
	//float scaledEY = e.y / static_cast<double> (getHeight());
    gPositionRaw[0] = (double) e.x;
    gPositionRaw[1] = (double) e.y;
    flagMouseUp = false;

}

void MainComponent::mouseDrag(const MouseEvent &e)
{
    
    if (e.mouseWasDraggedSinceMouseDown())
    {
//        Logger::getCurrentLogger()->outputDebugString("Mouse dragged!");

        float scaledEX = e.x / static_cast<double> (getWidth());
        float scaledEY = e.y / static_cast<double> (getHeight());

        gPositionRaw[0] = e.x;
        gPositionRaw[1] = e.y;
        
        

        if (scaledEX <= 0.0)
        {
            scaledEX = 0.0;
        }
        else if (scaledEX >= 1.0)
        {
            scaledEX = 1.0;
        }
        
        if (scaledEY <= 0.0)
        {
            scaledEY = 0.0;
        }
        else if (scaledEY >= 1.0)
        {
            scaledEY = 1.0;
        }
        
        gPosition[0] = scaledEX;
        gPosition[1] = scaledEY;

    //	Logger::getCurrentLogger()->outputDebugString("x: (" + String(gPosition[0]) + ") y: (" + String(gPosition[1]) + ")");

        double idx_map = linearMapping(1.0, 0.0, static_cast<double> (numMasses), 0.0, gPosition[1]);
        idx_mass = floor(idx_map);
        if (idx_mass == numMasses)
        {
            idx_mass = numMasses - 1.0;
        }
        for (int j = 0; j < numMasses; ++j)
        {
            if (idx_mass == j)
            {
                mass_spring_dampers[j]->setBow(true);
                mass_spring_dampers[j]->activate();
            }
            else
            {
                mass_spring_dampers[j]->setBow(false);
            }
        }


        //Logger::getCurrentLogger()->outputDebugString("xDiff: (" + String(gPosition[0] - gPositionPrev[0]) + ")");


        double maxVb = 0.4;
        double Vb = linearMapping(-0.08, 0.08, -maxVb, maxVb, gPosition[0] - gPositionPrev[0]);
        if (Vb > -0.02 && Vb < 0.02) // limit Vb to avoid small noise during small displacements
        {
            Vb = 0.0;
        }
        if (Vb > 0.4) // limit Vb to avoid small noise during small displacements
        {
            Vb = 0.4;
        }

//        Logger::getCurrentLogger()->outputDebugString("Vb: (" + String(Vb) + ")");
        //Logger::getCurrentLogger()->outputDebugString("FB: (" + String(mass_spring_dampers[idx]->getFb()) + ")");

        mass_spring_dampers[idx_mass]->setVb(Vb);

        gPositionPrev[0] = gPosition[0];
        gPositionPrev[1] = gPosition[1];




        //double maxFB = 1000;
        //double FB = linearMapping(-0.1, -30, 0, maxFB, gPosition[1]);
        //mass_spring_dampers[idx]->setFb(FB);
    }


}

void MainComponent::mouseUp(const MouseEvent &e)
{
//    Logger::getCurrentLogger()->outputDebugString("Mouse up!");

    flagMouseUp = true;
    
    gPositionRaw[0] = (double) e.x;
    gPositionRaw[1] = (double) e.y;
    
//    Logger::getCurrentLogger()->outputDebugString("mouseX: (" + String(gPositionRaw[0]) + ")");

    gPositionPrev[0] = gPosition[0];
    gPositionPrev[1] = gPosition[1];
    
    for (int j = 0; j < numMasses; ++j)
    {
        mass_spring_dampers[j]->setBow (false);
    }
}

void MainComponent::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& w)
{
	//Logger::getCurrentLogger()->outputDebugString("wheel_deltay: (" + String(w.deltaY) + ")");

	wheelDeltaY = wheelDeltaY + w.deltaY;
	if (wheelDeltaY < -5.0)
	{
		wheelDeltaY = -5.0;
	}
	else if (wheelDeltaY > 0.0)
	{
		wheelDeltaY = 0.0;
	}


//	Logger::getCurrentLogger()->outputDebugString("wheel_deltay: (" + String(wheelDeltaY) + ")");


	double maxFB = 1000;
	double FB = linearMapping(-5.0, 0.0, maxFB, 0.0, wheelDeltaY);

	Logger::getCurrentLogger()->outputDebugString("FB: (" + String(FB) + ")");

//	mass_spring_dampers[idx_mass]->setFb(FB); // update force on each mass..
    
    for (int j = 0; j < numMasses; ++j) // update force on all masses at once (better!)
    {
        mass_spring_dampers[j]->setFb(FB);
    }
    
    opa_level = FB/maxFB;
    if (opa_level < 0.1)
    {
        opa_level = 0.1;
    }
    else if (opa_level > 0.9)
    {
        opa_level = 0.9;
    }
}


