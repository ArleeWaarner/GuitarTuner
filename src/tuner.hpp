#pragma once

#include <portaudio.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <fstream>
#include <map>
#include <complex>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
#include <typeinfo>


#ifdef _WIN32
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

bool isSpacePressed() {
#ifdef _WIN32
    return _kbhit() && _getch() == ' ';
#else
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch == ' ';
#endif
}

const int INPUT_SAMPLE_RATE = 60000 ;
const int INPUT_FRAMES_PER_BUFFER = 4096;     // 4096 frames in 1 buffer

const int INPUT_WAIT = 5; 
const int WINDOW_WIDTH = 26400;
const int WINDOW_HEIGHT = 1080;
const float GRAPH_AMPLITUDE = 1.0f;
int tuneRange = 5;
int fftFlag = 0;

using namespace std;


float* in = new float;
int nb_call=-1;
std::ofstream inputFile;
std::ofstream outputFile;
float a[2], b[3], mem1[4], mem2[4];
double stringFreq;
float cutoff;

// These two functions aims to implement a second order low pass filter (for noise reduction purposes)
void computeSecondOrderLowPassParameters( float srate, float f, float *a, float *b );
float processSecondOrderFilter( float x, float *mem, float *a, float *b );

// Callback function that handles the input signal
int audioInputCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

// Process the FFT algorithm on the discrete input signal
void fft(vector<complex<double>>& signal);
vector<complex<double>> computeFFT(const vector<complex<double>>& input);

// Reads input file
int readInputFile(vector<complex<double>>& input);

// Generate the frequency spectrum to 1600 Hz (if -f or --ftt flag added)
int generateSpectrum(vector<complex<double>> result);


// Handles the recording of the signal 
int recording(std::string currentString);

// Handles the analyze of the recorded signal
int analyzing();







