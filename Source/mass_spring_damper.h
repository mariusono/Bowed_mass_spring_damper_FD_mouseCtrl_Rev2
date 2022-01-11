/*
  ==============================================================================

    mass_spring_damper.h
    Created: 17 Nov 2021 8:08:51pm
    Author:  Marius Onofrei

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class mass_spring_damper  : public juce::Component
{
public:
    mass_spring_damper(double freq, double fs);
    ~mass_spring_damper() override;

    
    void paint (Graphics&) override;
    void resized() override;
            
    double process();
    void newtonRaphson();
    //void updateStates();

    void setBow (bool val) { isBowing = val; };
    void setVb (double val) { vB = val; }
	void setFb(double val) { FB = val; }
    void setDamping(double val) { c_damp = val; }
    void setFrParam(double val) { a = val; }
    void setStickFact(double val) { stickFact = val; }

	double getDamping() { return c_damp; }
	double getVb() { return vB; }
    double getFb() { return FB; }
    double getFrParam() { return a; }
    double getStickFact() { return stickFact;  }

    bool isActive() { return active; };
    void activate() { active = true; };
    void deactivate() { active = false; };


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (mass_spring_damper)
    
    // init params
    double fs, freq, mass, k_stiff, c_damp, sig0, vB, A, tol, a, T, w0;
	double FB;

    double stickFact;
    
    // process params
	double F_fr;
	
    // NR params
    double eps, numerator, denominator;
	
	// State params
    double u_n, u_nm1, u_np1; // velocity and displacement of mass

    
//    gamma, k, s0, s1, B, kappa, h, N, lambdaSq, muSq, kOh, gOh, a, BM, Vb, Fb, pickup, tol, q, qPrev, bp, b, eps;
    bool isBowing = false;
//    std::vector<double> u, uPrev, uNext;
    bool active = false;
    
    unsigned long countGlobal;
//    unsigned long t = 0;
};
