#pragma once
#include <vector>
#include "CSetting.h"
#include "ISettingCallback.h"
#include "../XRThreads/CriticalSection.h"
#include "../XRThreads/SingleLock.h"

class CSettingManager;


//template <typename T>
//using List = std::vector < CSettingIpml<T> > ;

/*!*\class CSetting*/
template<typename T>
class CSettingIpml : public CSetting {
	typedef std::vector<T> SettingList;
	//For future implementation, to generate settings at runtime
	//typedef void(*OptionsFiller) (const CSettingIpml* setting, std::vector<T> &list, T& currentValue);

public:
	CSettingIpml(const std::string &id, CSettingManager* manager = nullptr);
	CSettingIpml(const std::string &id, const CSettingIpml& setting);
	virtual ~CSettingIpml() {};

	virtual bool Deserialize(const XMLNode *node, bool update = false);

	bool IsList() { return m_bList; }

	virtual T GetValue() { XR::CSingleLock lck(m_critical); return m_Value; }
	virtual bool SetValue(T value);
	virtual bool SetDefault(T value);
	virtual bool Equals(const T &value) {
		return (m_Value == value);
	}

	virtual bool CheckValidity(const T &value) const = 0;
	virtual void Reset() { SetValue(m_DefaultValue); }

protected:	
	virtual void Copy(const CSettingIpml& setting);

	T m_Value;
	T m_DefaultValue;
	SettingList m_vSettingList;
};

/*!
* \class CSettingInt
*/
class CSettingInt : public CSettingIpml < int > {
public:
	CSettingInt(const std::string &id, CSettingManager *settingsManager = NULL);
	CSettingInt(const std::string &id, const CSettingInt &setting);
	CSettingInt(const std::string &id, int value, CSettingManager *settingsManager = NULL);
	CSettingInt(const std::string &id, int value, int minimum, int step, int maximum, CSettingManager *settingsManager = NULL);
	virtual ~CSettingInt() {};

	virtual bool FromString(const std::string &value) override;
	virtual std::string ToString() override;
	virtual bool CheckValidity(const int &value) const override;
	virtual bool Deserialize(const XMLNode *node, bool update = false) override;
	virtual bool Serialize(XMLNode *node);

protected:
	virtual void Copy(const CSettingInt& setting) ;

private:
	int m_min;
	int m_max;
	int m_step;
};

/*!*\class CSettingFloat */
class CSettingFloat : public CSettingIpml < float > {
public:
	CSettingFloat(const std::string &id, CSettingManager *settingsManager = NULL);
	CSettingFloat(const std::string &id, const CSettingFloat &setting);
	CSettingFloat(const std::string &id, float value, CSettingManager *settingsManager = NULL);
	CSettingFloat(const std::string &id, float value, float minimum, float step, float maximum, CSettingManager *settingsManager = NULL);
	virtual ~CSettingFloat() {};

	virtual bool CheckValidity(const float &value) const override;
	virtual bool Deserialize(const XMLNode *node, bool update = false) override;
	virtual bool FromString(const std::string &value) override;
	virtual std::string ToString() override;
	virtual bool Serialize(XMLNode *node);


protected:
	virtual void Copy(const CSettingFloat& setting);

private:
	float m_min;
	float m_max;
	float m_step;
};

/*!*\class CSettingBool */
class CSettingBool : public CSettingIpml < bool > {
public:
	CSettingBool(const std::string &id, CSettingManager *settingsManager = NULL);
	CSettingBool(const std::string &id, const CSettingBool &setting);
	CSettingBool(const std::string &id, bool value, CSettingManager *settingsManager = NULL);
	virtual ~CSettingBool() {};

	virtual bool CheckValidity(const bool &value) const override;
	virtual bool Deserialize(const XMLNode *node, bool update = false) override;
	virtual bool FromString(const std::string &value) override;
	virtual std::string ToString() override;
	virtual bool Serialize(XMLNode *node);

protected:
	virtual void Copy(const CSettingBool& setting);
};

/*!*\class CSettingString */
class CSettingString : public CSettingIpml < std::string > {
public:
	CSettingString(const std::string &id, CSettingManager *settingsManager = NULL);
	CSettingString(const std::string &id, const CSettingString &setting);
	CSettingString(const std::string &id, std::string value, CSettingManager *settingsManager = NULL);
	virtual ~CSettingString() {};

	virtual bool CheckValidity(const std::string &value) const override;
	virtual bool Deserialize(const XMLNode *node, bool update = false) override;
	virtual bool FromString(const std::string &value) override;
	virtual std::string ToString() override;
	virtual bool Serialize(XMLNode *node);

protected:
	virtual void Copy(const CSettingString& setting);
};
