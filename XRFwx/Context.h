#pragma once
#include <memory>
#include "XRFramework/render/D3D10Enumeration.h"
#include "XRCommon/settings/helpers/Monitors.h"
#include "wxDX10Display.h"
class MainFrame;
class CDX10SystemRenderer;
class CDX10FrameRenderer;
class MainFrame;
class wxWindow;
class CPlayer;
class wxDX10Display;

struct Context
{
	std::unique_ptr<CMonitors> monitors;

	// Things that should probably be in some sort of UI-context-model
	wxWindow *parent = nullptr;
	wxWindow *previousFocus = nullptr;
	wxWindow *videoSlider = nullptr;

	MainFrame *frame = nullptr;
	wxDX10Display *videoDisplay = nullptr;
   CDX10FrameRenderer * frameRenderer = nullptr;
   CPlayer *player;

	Context()  : monitors(std::make_unique<CMonitors>()) {};
	~Context() {};
};

