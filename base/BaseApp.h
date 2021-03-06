#pragma once
#include <Windows.h>
#include <chrono>
#include "Settings.h"
#include "..\resources\ResourceContainer.h"
#include "..\gamestates\GameStateMachine.h"
#include "core\base\EventStream.h"
#include "core\base\system_info.h"
#include <core\base\ShortcutsHandler.h>
#include <thread>
#include "core\plugin\Plugin.h"
#include <core\base\InputStates.h>
#include "..\editor\GameEditor.h"
#include "StepTimer.h"

namespace ds {

	class BaseApp {

		struct DebugInfo {
			bool createReport;
			bool updated;
			bool showGameStateDialog;
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
		virtual void onShutdown() {}
		void buildFrame();
		bool isLoading() const {
			return _loading;
		}
		void sendOnChar(char ascii, unsigned int state);
		void sendButton(int button, int x, int y, bool down);
		void shutdown();
		bool isRunning() const;
		void sendKeyUp(WPARAM virtualKey);
		void sendKeyDown(WPARAM virtualKey);
		void addShortcut(const char* label, char key, uint32_t eventType);
	protected:
		void addGameState(GameState* gameState);
		void pushState(const char* name);
		void popState();
		//void deactivate(const char* name);
		void connectGameStates(const char* firstStateName, int outcome, const char* secondStateName);
		virtual void prepare(Settings* settings) = 0;
		virtual const char* getTitle() = 0;
		virtual void init() {}
		virtual void OnChar(uint8_t ascii) {}
		virtual void OnButtonDown(int button, int x, int y) {}
		virtual void OnButtonUp(int button, int x, int y) {}
		virtual bool loadContent() {
			return true;
		}
	private:
		void saveReport();
		void tick(double elapsed);
		void renderFrame();
		Settings _settings;
		HINSTANCE hInstance;
		HWND m_hWnd;
		bool _loading;
		bool _running;
		bool _alive;
		GameStateMachine* _stateMachine;
		ShortcutsHandler* _shortcuts;
		GameEditor* _editor;
		KeyStates _keyStates;
		ButtonState _buttonState;		
		std::thread _thread;
		SystemInfo _systemInfo;
		DebugInfo _debugInfo;
		StepTimer _stepTimer;
	};

}