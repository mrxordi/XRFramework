#pragma once

class CSetting;

class ISettingCallback
{
public:
	virtual ~ISettingCallback();

	//callback fired when setting state is going to change
	virtual bool OnSettingChanging(CSetting* setting) = 0;

	//callback fired when setting has been changed
	virtual bool OnSettingChanged(CSetting* setting) = 0;

};

