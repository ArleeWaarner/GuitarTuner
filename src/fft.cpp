#include <iostream>
#include <complex>
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <SFML/Window.hpp>
#include <string>

using namespace std;

// Dimensions for displaying the frequency spectrum
const int WINDOW_WIDTH = 26400;
const int WINDOW_HEIGHT = 1080;
const float GRAPH_AMPLITUDE = 1.0f;

int generateSpectrum(vector<complex<double>> result);

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
    ifstream file("outputRecorder.txt");
    if (!file.is_open()) {
        cerr << "Error opening file." << endl;
        return 1;
    }
    string line;
    while (getline(file, line)) {
        double line_double = stod(line);
        input.push_back(line_double);
    }

    file.close();
    return 0;
}

int main() {

    // Samples per second
    const double samplingRate = 30000.0; 

    vector<complex<double>> input;

    readInputFile(input);

    cout << "FFT processing..." << endl;

    vector<complex<double>> result = computeFFT(input);

    int paddedNumSamples = result.size();

    // Outputs the frequency of the maximum amplitude
    int k=1;
    double freqMax=0.0;
    double max=0.0;
    for (auto& value : result) {
        if(max<abs(value)) {
            max = abs(value);
            freqMax = k*(samplingRate/paddedNumSamples);
        }
        k++;;
    }

    cout << "freqMax = " << freqMax << " Hz" <<endl;

    // Displays frequency spectrum
    generateSpectrum(result);

    return 0;
}


int generateSpectrum(vector<complex<double>> result) {

   int POINTS_COUNT = result.size();
    // Create the window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Graph Drawing");

    // Create a vector to store the points for the graph
    vector<sf::Vertex> points;

    // Load the font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        // Error handling: Display a message and return
        cout << "Error loading font!" << endl;
        return 1;
    }

    // Generate the points for the graph
    for (int i = 0; i < POINTS_COUNT; ++i) {
        float x = 50+(static_cast<float>(i)* (WINDOW_WIDTH))/ POINTS_COUNT;
        float y = 700 + GRAPH_AMPLITUDE * (-abs(result[i])); 
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
        sf::Text label(to_string(i - 50), font, 12);
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
        sf::Text label(to_string(300 - i), font, 12);
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
            sf::Text label(to_string((i+k*6) - 50), font, 12);
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
            sf::Text label(to_string(700 - i), font, 12);
            label.setFillColor(sf::Color::Magenta);
            label.setPosition(17, i-8);
            window.draw(label);
        }

        // Draw the points
        window.draw(&points[0], points.size(), sf::Points);

        // Display the contents of the window
        window.display();
    }


    return 0;
}
