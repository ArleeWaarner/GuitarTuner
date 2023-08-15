#include "tuner.hpp"


// main thread
int main(int argc, char* argv[]){

    int precisionFlag = 0;

    // Check for command-line flags
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ) {
                cout << "Usage: ./new [option] [optionValue] ..." << endl;
                cout << "Options:" << endl;
                cout << "-h, --help                  Print this message and exit." << endl;
                cout << "-p, --precision             Set the tune range (in cent)." << endl;
                cout << "-f, --fft                   Display frequency spectrum." << endl;
                return 1;
            } 
            else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--precision") == 0) {
                if(argc > i+1){
                    precisionFlag = 1;
                    try {
                        tuneRange = stoi(argv[i+1]);
                        cout << "Precision set to " << tuneRange << " cents"<< endl;
                    }
                    catch (const std::exception&) {
                        cout << "Wrong precision value entered" << endl;
                        return 1;
                    }
                }
            }
            else if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fft") == 0) {
                fftFlag = 1;
            } 
        }
    } 


    if(precisionFlag==0) {
        cout << "No precision value set, default precision : " << tuneRange << " cents" << endl;
    }

    std::string userInput="";  
    std::string currentString="";  

    while(true){

        if(currentString=="") {
            cerr << "What string do you want to tune ? ('E2', 'A2', 'D3', 'G3', 'B3', 'E4')" <<endl;      
            std::cin >> std::ws;
            std::getline(std::cin, userInput);
            currentString = userInput;
        }

        int res1 = recording(currentString);
        if(res1>0) {
            analyzing();
        }

        cerr << "What string do you want to tune ? ('E2', 'A2', 'D3', 'G3', 'B3', 'E4')" <<endl;
        cerr << "Press spacebar to keep tuning the same string" << endl;   
        while (true) {
            if (isSpacePressed()) {
                cerr << "Continuing to tune the same string" << std::endl;
                break; // Exit the loop when space is detected
            }
            else {
                cerr << "Changing string to tune" << std::endl;
                currentString="";
                break;
            }
        }
    }

    return 0;
}


int recording(std::string currentString) {

    cerr << "Guitar tuner starting..." << std::endl;

    // Creating map to link notes and frequencies
    std::map<std::string, float> stringsFreq;
    stringsFreq.insert(std::pair<std::string, float>("E2", 82.41));
    stringsFreq.insert(std::pair<std::string, float>("A2", 110.8));
    stringsFreq.insert(std::pair<std::string, float>("D3", 146.83));
    stringsFreq.insert(std::pair<std::string, float>("G3", 196.0));
    stringsFreq.insert(std::pair<std::string, float>("B3", 246.94));
    stringsFreq.insert(std::pair<std::string, float>("E4", 329.63));

    // Creating map to link notes and cutoff frequencies for low-pass filter
    std::map<std::string, float> cutoffFreq;
    cutoffFreq.insert(std::pair<std::string, float>("E2", 100.0));
    cutoffFreq.insert(std::pair<std::string, float>("A2", 150.0));
    cutoffFreq.insert(std::pair<std::string, float>("D3", 171.0));
    cutoffFreq.insert(std::pair<std::string, float>("G3", 225.0));
    cutoffFreq.insert(std::pair<std::string, float>("B3", 285.0));
    cutoffFreq.insert(std::pair<std::string, float>("E4", 400.0));

    std::cout << "Tuner set for string : " << currentString << std::endl;

    stringFreq = stringsFreq[currentString];
    cutoff = cutoffFreq[currentString];

    if(stringFreq>0){
        std::cout << "Tuner set to tune string at : " << stringFreq << " Hz" << std::endl;
        std::cout << "The low-pass filter will cut off at : " << cutoff << " Hz" << std::endl;
    }

    else {
        std::cerr << "Error : no such string to be set : " << currentString << std::endl;
        return -1;
    }

    // Open the input file (provide the desired file name)
    inputFile.open("input.txt");

    // Check if the file is opened successfully
    if (!inputFile.is_open()) {
        std::cout << "Failed to open the input file." << std::endl;
        return -1;
    }

    // Open the output file (provide the desired file name)
    outputFile.open("output.txt");

    // Check if the file is opened successfully
    if (!outputFile.is_open()) {
        std::cout << "Failed to open the output file." << std::endl;
        return -1;
    }

    PaError inputErr, outputErr;
    recorded = new float[INPUT_FRAMES_PER_BUFFER];

    // Initialize PortAudio
    inputErr = Pa_Initialize();
    if (inputErr != paNoError)
    {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(inputErr) << std::endl;
        return -1;
    }

    int nb_device = Pa_GetDeviceCount();

    if(nb_device<0){
        std::cerr << "Error getting devices count" << std::endl;
        return -1;
    }
    else if(nb_device==0) {
        std::cerr << "There is no available device on this machine" << std::endl;
        return -1;
    }

    int device_input = 0;
    int device_output = 1;

    PaStreamParameters inputParameters;

    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = 1;
    inputParameters.device = device_input;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(device_input)->defaultLowInputLatency;

    // I N P U T
    // Open a PortAudio stream with input
    PaStream *InputStream;
    inputErr = Pa_OpenStream(&InputStream, &inputParameters, nullptr, INPUT_SAMPLE_RATE, 
                               INPUT_FRAMES_PER_BUFFER, paNoFlag,audioInputCallback, nullptr);

    if (inputErr != paNoError)
    {
        std::cerr << "PortAudio stream opening failed: " << Pa_GetErrorText(inputErr) << std::endl;
        Pa_Terminate();
        return -1;
    }

    // Start the audio stream
    inputErr = Pa_StartStream(InputStream);
    if (inputErr != paNoError)
    {
        std::cerr << "PortAudio stream starting failed: " << Pa_GetErrorText(inputErr) << std::endl;
        Pa_CloseStream(InputStream);
        Pa_Terminate();
        return -1;
    }

    Pa_Sleep(INPUT_WAIT*1000);

    // Stop and close the audio stream
    inputErr = Pa_StopStream(InputStream);
    if (inputErr != paNoError)
    {
        std::cerr << "PortAudio stream stopping failed: " << Pa_GetErrorText(inputErr) << std::endl;
    }

    inputFile.close();

    inputErr = Pa_CloseStream(InputStream);
    if (inputErr != paNoError)
    {
        std::cerr << "PortAudio stream closing failed: " << Pa_GetErrorText(inputErr) << std::endl;
    }

    // Terminate PortAudio
    inputErr = Pa_Terminate();
    if (inputErr != paNoError)
    {
        std::cerr << "PortAudio termination failed: " << Pa_GetErrorText(inputErr) << std::endl;
        return -1;
    }

    return 1;
}


int analyzing() {
    cout << "FFT result:" << endl;
    const double samplingRate = static_cast<double>(INPUT_SAMPLE_RATE); // Samples per second

    vector<complex<double>> input;

    readInputFile(input);

    vector<complex<double>> result = computeFFT(input);

    int paddedNumSamples = result.size();

    //Low-pass filter with custom cutoff frequency
    int fc = static_cast<int>(cutoff);
    for (int i = 0; i < paddedNumSamples; i++) {
        if(i*(samplingRate/paddedNumSamples)>fc) {
            result[i]=0.0;
        }
    }

    int k=1;
    double freqMax=0.0;
    double max=0.0;
    for (auto& value : result) {
        if(max<std::abs(value)) {
            max = std::abs(value);
            freqMax = k*(samplingRate/paddedNumSamples);
        }
        k++;;
    }

    cerr << "\n" << "freqMax = " << freqMax << " Hz" <<endl;

    double nbCents = 1200 * log2(freqMax / stringFreq);

    if(nbCents<tuneRange && nbCents>-tuneRange){
        cerr << "The string is tuned !" <<endl;
    }
    else if(nbCents>0) {
        cerr << "You are sharp by " << nbCents << " cents !" <<endl;

    }
    else if(nbCents<0) {
        cerr << "You are flat by " << nbCents << " cents !" <<endl;
    } 

    if(fftFlag>0) {
        generateSpectrum(result);
    }


    return 0;
}

static float inline maxi(float a, float b){
    return a > b ? a : b;
}

static float inline abso(float a){
    return a > 0 ? a : -a;
}

int audioInputCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *userData)
{
    float a[2], b[3], mem1[4], mem2[4];
    mem1[0] = 0; mem1[1] = 0; mem1[2] = 0; mem1[3] = 0;
    mem2[0] = 0; mem2[1] = 0; mem2[2] = 0; mem2[3] = 0;
    nb_call++;
    // Cast the input and output buffers to floating-point format
    float* in = const_cast<float*>(static_cast<const float*>(inputBuffer));

    recorded = in; 

    int dipSize = 100;
    printf("\r");

    float vol_l = 0;
    float vol_r = 0;

    std::cout.rdbuf(inputFile.rdbuf());

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        recorded[i+nb_call*framesPerBuffer] = in[i];
        std::cout << in[i] << std::endl;
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


int audioOutputCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *userData){

    nb_call++;
    float* out = static_cast<float*>(outputBuffer);

    std::cout.rdbuf(outputFile.rdbuf());

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        out[i] = recorded[i+nb_call*framesPerBuffer];
        std::cout << out[i] << std::endl;
    }

    return paContinue;
}

// Recursive FFT function
void fft(vector<complex<double>>& signal) {
    int n = signal.size();
    if (n <= 1) {
        return; // Base case: nothing to do
    }

    vector<complex<double>> even(n / 2);
    vector<complex<double>> odd(n / 2);

    // Split the signal into even and odd parts
    for (int i = 0; i < n / 2; ++i) {
        even[i] = signal[i * 2];
        odd[i] = signal[i * 2 + 1];
    }

    // Recursively compute FFT for even and odd parts
    fft(even);
    fft(odd);

    // Combine the results
    for (int k = 0; k < n / 2; ++k) {
        complex<double> t = polar(1.0, -2 * M_PI * k / n) * odd[k];
        signal[k] = even[k] + t;
        signal[k + n / 2] = even[k] - t;
    }
}

// Function to compute FFT for a given input sequence
vector<complex<double>> computeFFT(const vector<complex<double>>& input) {
    vector<complex<double>> signal = input;
    int n = signal.size();

    // Ensure the input size is a power of 2 for the FFT algorithm
    int power = 1;
    while (power < n) {
        power *= 2;
    }
    signal.resize(power, 0); // Zero-padding if necessary

    // Call the recursive FFT function
    fft(signal);

    return signal;
}

int readInputFile(vector<complex<double>>& input) {
    std::ifstream file("input.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    std::string line;
    int k=1;
    while (std::getline(file, line)) {
        // Process the line here
        double line_double = std::stod(line);
        input.push_back(line_double);
        k++;
    }

    file.close();
    return 0;
}

int generateSpectrum(vector<complex<double>> result) {


   int POINTS_COUNT = result.size();

    // Create the window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Graph Drawing");

    // Create a vector to store the points for the graph
    std::vector<sf::Vertex> points;

    // Load the font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        // Error handling: Display a message and return
        std::cout << "Error loading font!" << std::endl;
        return -1;
    }

    // Generate the points for the graph
    for (int i = 0; i < POINTS_COUNT; ++i) {
        float x = 50+(static_cast<float>(i)* (WINDOW_WIDTH))/ POINTS_COUNT;
        float y = 700 + GRAPH_AMPLITUDE * (-std::abs(result[i])); 
        points.emplace_back(sf::Vector2f(x, y), sf::Color::Blue);
    }


    // Create the axes lines with black color
    sf::VertexArray x_axis(sf::Lines, 2);
    x_axis[0].position = sf::Vector2f(0, 700);
    x_axis[0].color = sf::Color::Black;
    x_axis[1].position = sf::Vector2f(WINDOW_WIDTH, 700);
    x_axis[1].color = sf::Color::Black;

    sf::VertexArray y_axis(sf::Lines, 2);
    y_axis[0].position = sf::Vector2f(50, 0);
    y_axis[0].color = sf::Color::Black;
    y_axis[1].position = sf::Vector2f(50, WINDOW_HEIGHT);
    y_axis[1].color = sf::Color::Black;

    // Draw graduated markings (ticks) on the x-axis with black color
    sf::VertexArray x_ticks(sf::Lines);
    for (int i = 50; i <= 2000; i += 44)
    {
        x_ticks.append(sf::Vertex(sf::Vector2f(i, 695), sf::Color::Black));
        x_ticks.append(sf::Vertex(sf::Vector2f(i, 705), sf::Color::Black));

        // Add numeric labels
        sf::Text label(std::to_string(i - 50), font, 12);
        label.setFillColor(sf::Color::Blue);
        label.setPosition(i - 5, 710);
        window.draw(label);
    }

    // Draw graduated markings (ticks) on the y-axis with black color
    sf::VertexArray y_ticks(sf::Lines);
    for (int i = 50; i <= 1050; i += 50)
    {
        y_ticks.append(sf::Vertex(sf::Vector2f(45, i), sf::Color::Black));
        y_ticks.append(sf::Vertex(sf::Vector2f(55, i), sf::Color::Black));

        // Add numeric labels
        sf::Text label(std::to_string(300 - i), font, 12);
        label.setFillColor(sf::Color::Blue);
        label.setPosition(370, i - 5);
        window.draw(label);
    }

    // Main loop
    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear the window
        window.clear(sf::Color::White);

        // Draw the axes
        window.draw(x_axis);
        window.draw(y_axis);

        // Draw the graduated markings (ticks)
        window.draw(x_ticks);
        window.draw(y_ticks);

        // Draw graduated markings (ticks) on the x-axis with black color
        sf::VertexArray x_ticks(sf::Lines);
        int k=0;
        for (int i = 50; i <= 1900; i += 44)
        {
            x_ticks.append(sf::Vertex(sf::Vector2f(i, 695), sf::Color::Black));
            x_ticks.append(sf::Vertex(sf::Vector2f(i, 705), sf::Color::Black));

            // Add numeric labels
            sf::Text label(std::to_string((i+k*6) - 50), font, 12);
            label.setFillColor(sf::Color::Blue);
            label.setPosition(i - 5, 710);
            window.draw(label);
            k++;
        }

        // Draw graduated markings (ticks) on the y-axis with black color
        sf::VertexArray y_ticks(sf::Lines);
        for (int i = 50; i <= 1050; i += 50)
        {
            y_ticks.append(sf::Vertex(sf::Vector2f(715, i), sf::Color::Red));
            y_ticks.append(sf::Vertex(sf::Vector2f(725, i), sf::Color::Red));

            // Add numeric labels
            sf::Text label(std::to_string(700 - i), font, 12);
            label.setFillColor(sf::Color::Magenta);
            label.setPosition(17, i-8);
            window.draw(label);
        }

        // Draw the points
        window.draw(&points[0], points.size(), sf::Points);

        // Display the contents of the window
        window.display();
    }


    return 1;
}



void computeSecondOrderLowPassParameters( float srate, float f, float *a, float *b )
{
   float a0;
   float w0 = 2 * M_PI * f/srate;
   float cosw0 = cos(w0);
   float sinw0 = sin(w0);
   //float alpha = sinw0/2;
   float alpha = sinw0/2 * sqrt(2);

   a0   = 1 + alpha;
   a[0] = (-2*cosw0) / a0;
   a[1] = (1 - alpha) / a0;
   b[0] = ((1-cosw0)/2) / a0;
   b[1] = ( 1-cosw0) / a0;
   b[2] = b[0];
}

float processSecondOrderFilter( float x, float *mem, float *a, float *b )
{
    float ret = b[0] * x + b[1] * mem[0] + b[2] * mem[1]
                         - a[0] * mem[2] - a[1] * mem[3] ;

        mem[1] = mem[0];
        mem[0] = x;
        mem[3] = mem[2];
        mem[2] = ret;

        return ret;
}










