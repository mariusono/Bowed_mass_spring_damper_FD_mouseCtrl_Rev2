/*
  ==============================================================================

    mass_spring_damper.cpp
    Created: 17 Nov 2021 8:08:51pm
    Author:  Marius Onofrei

  ==============================================================================
*/

#include <JuceHeader.h>
#include "mass_spring_damper.h"

//==============================================================================
mass_spring_damper::mass_spring_damper (double freq, double fs) : fs (fs), freq (freq)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    T = 1/fs; // time step
    
    w0 = 2*double_Pi*freq;

    mass = 1;
    k_stiff = w0 * w0 * mass;
    c_damp = 1; // Damping parameter ! important! 
	sig0 = c_damp / (2*mass);

    // Bowing params
//	FB = 1000;
	FB = 100;
//	FB = 5;
	vB = 0.1;
    a = 300.0; // friction law free parameter (1/m^2) (a) % decreasing sig increases the stick time
//    A = sqrt(2 * a) * exp(0.5);
    tol = 1e-12;
    stickFact = 1;
    
    // param init:
    u_n = 0;
    u_nm1 = 0;
    u_np1 = 0;
}

mass_spring_damper::~mass_spring_damper()
{
	
}

double mass_spring_damper::process()
{
	newtonRaphson(); // this updates F_fr.. maybe I should have a clear return ?

	sig0 = c_damp / (2 * mass);
	
	u_np1 = u_n * (2 - w0 * w0 * T * T)/(1 + sig0 * T) - u_nm1 * (1 - sig0 * T)/(1 + sig0*T) - F_fr * T * T / (mass + mass * sig0 * T);
	    
	// update states	
    u_nm1 = u_n;
    u_n = u_np1;

    if (isBowing == true)
    {
        countGlobal = 0;
    }
    
    if (!isBowing)
    {
        ++countGlobal;
        if (countGlobal > fs * 10)
        {
            deactivate(); // do i need this ? not if i have damping on..  // this doesn't do anything now..
        }
    }
    return u_n;
}
	

//void mass_spring_damper::updateStates()
//{
//    u_nm1 = u_n;
//    u_n = u_np1;
//}

void mass_spring_damper::newtonRaphson()
{
    eps = 1;
    int count = 1;
    double v_rel_last = 0;
	double b = w0 * w0 * u_n - 2 / (T * T) * (u_n - u_nm1) + (2 / T + 2 * sig0) * vB;
//    Logger::getCurrentLogger()->outputDebugString("vB: (" + String(vB) + ")");

//    double b_new = 0;
    while ((eps > tol) && (count < 99))
    {
        numerator =  (FB/mass) * sqrt(2 * a) * exp(0.5) * v_rel_last * exp(-a * v_rel_last * v_rel_last) + ((2/T) + 2*sig0)*v_rel_last + b;
        denominator = (FB/ mass) * sqrt(2 * a) * exp(0.5) * exp(-a * v_rel_last * v_rel_last) * (1 - 2 * a * v_rel_last * v_rel_last) + ((2/T) + 2*sig0);
		
        double v_rel_new = v_rel_last - numerator/denominator;

        eps = abs(v_rel_new-v_rel_last);
        v_rel_last = v_rel_new;
        count = count + 1;
    }
    if (isBowing == true)
    {
        F_fr = stickFact * FB * sqrt(2 * a) * exp(0.5) * v_rel_last * exp(-a * v_rel_last * v_rel_last);
    }else
    {
        F_fr = 0;
    }
//    std::cout << FB << std::endl;
}

void mass_spring_damper::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    
    // clear the background
     g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
     
     // choose your favourite colour
     g.setColour(Colours::cyan);
     
     // draw the state
//     g.strokePath(visualiseState (g, 100), PathStrokeType(2.0f));


//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (juce::Colours::white);
//    g.setFont (14.0f);
//    g.drawText ("mass_spring_damper", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
}

void mass_spring_damper::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
