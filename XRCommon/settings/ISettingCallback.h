#pragma once

class CSetting;

class ISettingCallback
{
public:
	virtual ~ISettingCallback() {};

	//callback fired when setting state is going to change
	virtual bool OnSettingChanging(const CSetting* setting) = 0;

	//callback fired when setting has been changed
	virtual void OnSettingChanged(const CSetting* setting) = 0;

};

