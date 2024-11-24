MAIN_SRC = main.cpp

app: $(MAIN_SRC) olcPixelGameEngine.h
	g++ -o app.exe $(MAIN_SRC) -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -lstdc++fs -static -std=c++17
