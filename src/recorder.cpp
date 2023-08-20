#include <portaudio.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <fstream>
#include <map>

const int INPUT_SAMPLE_RATE = 30000 ;
const int OUTPUT_SAMPLE_RATE = 30000 ;         // 30000 captures per second (1 frame) (30 kHz)
const int INPUT_FRAMES_PER_BUFFER = 4096;     // 4096 frames in 1 buffer
const int OUTPUT_FRAMES_PER_BUFFER = 4096;     

int INPUT_WAIT = 5;                            // Default recording duration 
int* OUTPUT_WAIT = &INPUT_WAIT;                    

float* recorded;
int nb_call=-1;
std::ofstream inputFile;
std::ofstream outputFile;

using namespace std;

// Returns the max out of a and b 
static float inline maxi(float a, float b){
    return a > b ? a : b;
}

// Returns the absolute value of float a 
static float inline abso(float a){
    return a > 0 ? a : -a;
}


// Callback function that handles the input signal
int audioInputCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *userData)
{
    nb_call++;
    // Cast the input and output buffers to floating-point format
    float* in = const_cast<float*>(static_cast<const float*>(inputBuffer));

    recorded = in; 

    int dipSize = 100;
    printf("\r");

    float vol_l = 0;
    float vol_r = 0;

    cout.rdbuf(inputFile.rdbuf());

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        recorded[i+nb_call*framesPerBuffer] = in[i];
        cout << in[i] << endl;
    }

    for(unsigned long i = 0; i < framesPerBuffer; i+=2){
        vol_l = maxi(vol_l,abso(in[i]));
        vol_r = maxi(vol_r,abso(in[i+1]));
    }

    for(int i=0;i<dipSize;i++){
        float barProportion = i / (float)dipSize;
        if (barProportion <= vol_l && barProportion <= vol_r) {
            printf("█");
        }
        else if(barProportion <= vol_l) {
            printf("▀");
        }      
        else if(barProportion <= vol_r) {
            printf("▄");
        }
        else{
            printf(" ");
        }
    }

    fflush(stdout);
    return 0;
}


// Callback function that handles the output signal
int audioOutputCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *userData){

    nb_call++;
    float* out = static_cast<float*>(outputBuffer);

    cout.rdbuf(outputFile.rdbuf());

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        out[i] = recorded[i+nb_call*framesPerBuffer];
        cout << out[i] << endl;
    }

    return paContinue;
}

int main()
{

    // Prompt the user for input : recording duration
    cout << "Please the recording time (in seconds) : ";

    // Read the user input from standard input (keyboard) and store it in the variable
    cin >> INPUT_WAIT;

    size_t initialSize = (INPUT_WAIT+1)*INPUT_SAMPLE_RATE;
    recorded = new float[initialSize];

    // Open the input file 
    // Each sample of the recorded audio will be stored in the file 'inputRecorder.txt' (Discretization)
    inputFile.open("inputRecorder.txt");

    // Check if the file is opened successfully
    if (!inputFile.is_open()) {
        cout << "Failed to open the input file." << endl;
        return 1;
    }

    // Open the output file 
    // Each sample of the played audio will be stored in the file 'outputRecorder.txt' 
    outputFile.open("outputRecorder.txt");

    // Check if the file is opened successfully
    if (!outputFile.is_open()) {
        cout << "Failed to open the output file." << endl;
        return 1;
    }

    PaError inputErr, outputErr;

    // Initialize PortAudio
    inputErr = Pa_Initialize();
    if (inputErr != paNoError)
    {
        cerr << "PortAudio initialization failed: " << Pa_GetErrorText(inputErr) << endl;
        return 1;
    }

    int nb_device = Pa_GetDeviceCount();

    if(nb_device<0){
        cerr << "Error getting devices count" << endl;
        return 1;
    }
    else if(nb_device==0) {
        cerr << "There is no available device on this machine" << endl;
        return 1;
    }

    int device_input = 0;
    int device_output = 1;

    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;

    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = 1;
    inputParameters.device = device_input;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(device_input)->defaultLowInputLatency;

    memset(&outputParameters, 0, sizeof(outputParameters));
    outputParameters.channelCount = 1;
    outputParameters.device = device_output;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(device_input)->defaultLowInputLatency;



    // I N P U T
    // Open a PortAudio stream with input

    PaStream *InputStream;
    inputErr = Pa_OpenStream(&InputStream, &inputParameters, nullptr, INPUT_SAMPLE_RATE, 
                               INPUT_FRAMES_PER_BUFFER, paNoFlag,audioInputCallback, nullptr);

    if (inputErr != paNoError)
    {
        cerr << "PortAudio stream opening failed: " << Pa_GetErrorText(inputErr) << endl;
        Pa_Terminate();
        return 1;
    }

    // Start the audio stream
    inputErr = Pa_StartStream(InputStream);
    if (inputErr != paNoError)
    {
        cerr << "PortAudio stream starting failed: " << Pa_GetErrorText(inputErr) << endl;
        Pa_CloseStream(InputStream);
        Pa_Terminate();
        return 1;
    }

    Pa_Sleep(INPUT_WAIT*1000);

    // Stop and close the audio stream
    inputErr = Pa_StopStream(InputStream);
    if (inputErr != paNoError)
    {
        cerr << "PortAudio stream stopping failed: " << Pa_GetErrorText(inputErr) << endl;
    }

    inputFile.close();

    inputErr = Pa_CloseStream(InputStream);
    if (inputErr != paNoError)
    {
        cerr << "PortAudio stream closing failed: " << Pa_GetErrorText(inputErr) << endl;
    }

    // Terminate PortAudio
    inputErr = Pa_Terminate();
    if (inputErr != paNoError)
    {
        cerr << "PortAudio termination failed: " << Pa_GetErrorText(inputErr) << endl;
        return 1;
    }


    // O U T P U T
    nb_call=-1;
    // Initialize PortAudio
    outputErr = Pa_Initialize();
    if (outputErr != paNoError)
    {
        cerr << "PortAudio initialization failed: " << Pa_GetErrorText(outputErr) << endl;
        return 1;
    }

    // Open a PortAudio stream with output
    PaStream *OutputStream;
    outputErr = Pa_OpenStream(&OutputStream, nullptr, &outputParameters, OUTPUT_SAMPLE_RATE, 
                               OUTPUT_FRAMES_PER_BUFFER, paNoFlag,audioOutputCallback, nullptr);

    if (outputErr != paNoError)
    {
        cerr << "PortAudio stream opening failed: " << Pa_GetErrorText(outputErr) << endl;
        Pa_Terminate();
        return 1;
    }

    // Start the audio stream
    outputErr = Pa_StartStream(OutputStream);
    if (outputErr != paNoError)
    {
        cerr << "PortAudio stream starting failed: " << Pa_GetErrorText(outputErr) << endl;
        Pa_CloseStream(OutputStream);
        Pa_Terminate();
        return 1;
    }

    Pa_Sleep(*OUTPUT_WAIT*1000);

    // Stop and close the audio stream
    outputErr = Pa_StopStream(OutputStream);
    if (outputErr != paNoError)
    {
        cerr << "PortAudio stream stopping failed: " << Pa_GetErrorText(outputErr) << endl;
    }

    outputFile.close();

    outputErr = Pa_CloseStream(OutputStream);
    if (outputErr != paNoError)
    {
        cerr << "PortAudio stream closing failed: " << Pa_GetErrorText(outputErr) << endl;
    }

    // Terminate PortAudio
    outputErr = Pa_Terminate();
    if (outputErr != paNoError)
    {
        cerr << "PortAudio termination failed: " << Pa_GetErrorText(outputErr) << endl;
        return 1;
    }


    if (remove("inputRecorder.txt") != 0) {
        cerr << "Failed to delete the input file" << endl;
    }


    if (remove("outputRecorder.txt") != 0) {
        cerr << "Failed to delete the output file" << endl;
    }


    return 0;
}







