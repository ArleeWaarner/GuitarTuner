CXX := g++
CXXFLAGS := -std=c++17
LIBS := -lportaudio -lsfml-graphics -lsfml-window -lsfml-system

all: recorder fft tuner

recorder: src/recorder.cpp
	$(CXX) $(CXXFLAGS) src/recorder.cpp -o recorder $(LIBS)

fft: src/fft.cpp
	$(CXX) $(CXXFLAGS) src/fft.cpp -o fft $(LIBS)

tuner: src/tuner.cpp
	$(CXX) $(CXXFLAGS) src/tuner.cpp -o tuner $(LIBS)

clean:
	rm -f recorder fft tuner

