#pragma once
#include <map>
#include <string>

#include "Variant.h"

class CUrlOptions
{
public:
  typedef std::map<std::string, CVariant> UrlOptions;

  CUrlOptions();
  CUrlOptions(const std::string &options, const char *strLead = "");
  virtual ~CUrlOptions();

  virtual void Clear() { m_options.clear(); m_strLead = ""; }

  virtual const UrlOptions& GetOptions() const { return m_options; }
  virtual std::string GetOptionsString(bool withLeadingSeperator = false) const;

  virtual void AddOption(const std::string &key, const char *value);
  virtual void AddOption(const std::string &key, const std::string &value);
  virtual void AddOption(const std::string &key, int value);
  virtual void AddOption(const std::string &key, float value);
  virtual void AddOption(const std::string &key, double value);
  virtual void AddOption(const std::string &key, bool value);
  virtual void AddOptions(const std::string &options);
  virtual void AddOptions(const CUrlOptions &options);
  virtual void RemoveOption(const std::string &key);

  virtual bool HasOption(const std::string &key) const;
  virtual bool GetOption(const std::string &key, CVariant &value) const;

protected:
  UrlOptions m_options;
  std::string m_strLead;
};
