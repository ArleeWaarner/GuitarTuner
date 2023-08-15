# YAGT

YAGT (Yet Another Guitar Tuner) is a guitar tuner designed to be simple and easy to understand. It was designed under MacOS and should easily be usable on any Unix OS. 
 
The tuner features a standalone mono recorder (easily switchable to stereo) and a standalone fft processer that displays the frequency spectrum (up to 1.6 kHz) of the previously recorded audio signal.

It uses an FFT algorithm on discrete audio signals. (Maybe not the most efficient solution on the market but easy to implement and good for signal understanding).
On top of tuning the ('E2', 'A2', 'D3', 'G3', 'B3', 'E4') strings, the tuner also offers the possibility to display the frequency spectrum of the recorded string sound.

# Installation
0. Make sure you have installed on your environment the following libraries : portaudio and sfml.
1. Clone the repo.
2. Run "make all" inside your local repo.
3. Choose to use the executable you need. If using "tuner", feel free to have a look at the options list below.

# Tuner's options
When running the exe file "tuner" :

-h, --help                  Print this message and exit.

-p, --precision             Set the tune range (in cent).    (Ex : --precision 10)

-f, --fft                   Display frequency spectrum.

