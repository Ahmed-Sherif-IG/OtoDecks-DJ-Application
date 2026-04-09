# OtoDecks DJ Application (C++ / JUCE)

## Overview
OtoDecks is a desktop DJ application developed in C++ using the JUCE framework, designed to simulate a professional dual-deck audio mixing environment. The application allows users to load, play, and manipulate audio tracks across two independent decks, while providing real-time waveform visualization and a playlist management system.

This project was developed as part of an Object-Oriented Programming coursework, demonstrating advanced concepts in audio processing, GUI development, and modular software design.

---

## Features

- 🎧 Dual-deck audio playback system (Deck 1 & Deck 2)  
- ▶️ Play, stop, and load audio tracks  
- 🔊 Volume control and playback speed adjustment  
- 📍 Track position control (seek functionality)  
- 🔄 Deck synchronization (match playback speed across decks)  
- 📊 Real-time waveform visualization  
- 📁 Playlist management system:
  - Add and remove tracks  
  - Load tracks into either deck  
- 🖱 Drag-and-drop file support  
- 🎨 Custom UI styling using JUCE LookAndFeel  

---

## Technologies Used

- **C++**
- **JUCE Framework (Audio & GUI development)**
- Object-Oriented Programming (OOP)
- Audio processing (resampling, transport control)
- File I/O (playlist persistence)

---

## How It Works

The application is structured using a modular object-oriented design, where each component has a clearly defined responsibility:

- `DJAudioPlayer` handles audio playback, including loading audio files, controlling playback, adjusting gain, and resampling for speed changes  
- `DeckGUI` represents each deck’s interface, providing controls such as play, stop, sync, and sliders for volume, speed, and position  
- `WaveformDisplay` visualizes the loaded track and updates playback position in real time 
- `PlaylistComponent` manages a list of tracks, allowing users to add, remove, and assign tracks to decks  
- `DeckLoadCellComponent` provides interactive buttons inside the playlist for loading tracks into specific decks 
- `MainComponent` integrates all components and mixes audio output from both decks using a mixer source 

The system uses JUCE’s audio pipeline, including `AudioTransportSource` and `ResamplingAudioSource`, to ensure smooth playback and real-time manipulation.

---

## How to Run

1. Open the project using the **Projucer** or your preferred C++ IDE (e.g., Visual Studio)  
2. Ensure JUCE modules are properly configured  
3. Build and run the project  
4. Load audio files using the “Load” button or drag-and-drop  
5. Control playback using the deck interfaces  

---

## Key Learnings

- Developed a full desktop application using the JUCE framework  
- Gained hands-on experience with real-time audio processing and playback systems  
- Applied object-oriented design principles to structure a large-scale project  
- Implemented custom GUI components and improved user experience with LookAndFeel customization  
- Learned how to synchronize multiple audio sources and manage state across components  
- Improved understanding of event-driven programming in C++  

---

## Future Improvements

- Add crossfader functionality for smoother mixing between decks  
- Implement beat matching and BPM detection  
- Enhance UI responsiveness and layout scaling  
- Support audio effects (EQ, filters, reverb)  
- Improve playlist features (search, sorting, metadata display)  

---