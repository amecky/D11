#pragma once
#include <Windows.h>
#include <chrono>
#include "Settings.h"
#include "..\resources\ResourceContainer.h"
#include "..\gamestates\GameStateMachine.h"

namespace ds {

	class BaseApp {

		struct KeyStates {
			uint8_t ascii;
			bool onChar;
			WPARAM keyPressed;
			bool keyDown;
			WPARAM keyReleased;
			bool keyUp;
		};

		struct ButtonState {
			int button;
			int x;
			int y;
			bool down;
			bool processed;
		};

	public:
		BaseApp();
		virtual ~BaseApp();
		void setInstance(const HINSTANCE hInst){
			hInstance = hInst;
		}
		bool prepare();
		void createWindow();
		virtual bool initialize() = 0;
		virtual void render() = 0;
		virtual void update(float dt) = 0;		
		void buildFrame();
		bool isLoading() const {
			return _loading;
		}
		void sendOnChar(char ascii, unsigned int state);
		void sendButton(int button, int x, int y, bool down);
	protected:
		void addGameState(GameState* gameState);
		void activate(const char* name);
		void deactivate(const char* name);
		void connectGameStates(const char* firstStateName, int outcome, const char* secondStateName);

		virtual const char* getTitle() = 0;
		virtual void init() {}
		virtual void OnChar(uint8_t ascii) {}
		virtual void OnButtonDown(int button, int x, int y) {}
		virtual void OnButtonUp(int button, int x, int y) {}
		virtual bool loadContent() {
			return true;
		}
	private:
		void tick();
		void renderFrame();

		HINSTANCE hInstance;
		HWND m_hWnd;
		float _dt;
		float _accu;
		float _fpsTimer;
		int _frames;
		int _fps;
		bool _loading;
		bool _running;
		Settings _settings;
		GameStateMachine* _stateMachine;
		KeyStates _keyStates;
		ButtonState _buttonState;
		bool _createReport;
		bool _updated;
		std::chrono::steady_clock::time_point _start, _now;
		float _ar[256];
		int _num;
	};

}