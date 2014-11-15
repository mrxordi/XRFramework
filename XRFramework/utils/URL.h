#pragma once
#include "utils/StdString.h"
#include "utils/URIUtils.h"
#include "utils/UrlOptions.h"

#undef SetPort // WIN32INCLUDES this is defined as SetPortA in WinSpool.h which is being included _somewhere_

class CURL {
public:
	enum _URLTYPE {
		UKNOWN = 0,
		FILE,
		LIBRARY,
		URL,
		PATH,
		FULLPATH
	}URLTYPE;
public:
	CURL(const CStdString& strURL1);
	CURL(void);
	virtual ~CURL(void);

	void Reset();
	void Parse(const CStdString& strURL);

	void SetFileName(const CStdString& strFileName);
	void SetHostName(const CStdString& strHostName);
	void SetUserName(const CStdString& strUserName);
	void SetPassword(const CStdString& strPassword);
	void SetProtocol(const CStdString& strProtocol);
	void SetOptions(const CStdString& strOptions);
	void SetProtocolOptions(const CStdString& strOptions);
	void SetProtocolOption(const CStdString &key, const CStdString &value);
	void SetOption(const CStdString &key, const CStdString &value);
	void SetPort(int port);

	const CStdString& GetFileName() const;
	const CStdString& GetHostName() const;
	const CStdString& GetUserName() const;
	const CStdString& GetPassWord() const;
	const CStdString& GetProtocol() const;
	const CStdString& GetOptions() const;
	const CStdString& GetProtocolOptions() const;
	void			  GetProtocolOptions(std::map<CStdString, CStdString> &options) const;
	const CStdString& GetFileType() const;
	const CStdString& GetShareName() const;
	const CStdString& GetDomain() const;

	const CStdString GetFileNameWithoutPath() const; /* return the filename excluding path */
	const CStdString GetTranslatedProtocol() const;
	CStdString		 GetWithoutFilename() const;

	std::string		 GetWithoutUserDetails(bool redact = false) const;

	CStdString GetOption(const CStdString &key) const;
	bool	   GetOption(const CStdString &key, CStdString &value) const;
	bool	   GetProtocolOption(const CStdString &key, CStdString &value) const;
	CStdString GetProtocolOption(const CStdString &key) const;
	int		   GetPort() const;
	CStdString Get() const;
	char	   GetDirectorySeparator() const;
	std::string		   GetRedacted() const;
	static std::string GetRedacted(const std::string& path);

	void RemoveOption(const CStdString &key);
	void RemoveProtocolOption(const CStdString &key);

	bool HasPort() const;
	bool HasProtocolOption(const CStdString &key) const;
	bool HasOption(const CStdString &key) const;


	bool IsLocal() const;
	bool IsLocalHost() const;
	static bool IsFileOnly(const CStdString &url); ///< return true if there are no directories in the url.
	static bool IsFullPath(const CStdString &url); ///< return true if the url includes the full path

	static std::string Decode(const std::string& strURLData);
	static std::string Encode(const std::string& strURLData);
	static CStdString TranslateProtocol(const CStdString& prot);

protected:
	int m_iPort;
	CStdString m_strHostName;
	CStdString m_strShareName;
	CStdString m_strDomain;
	CStdString m_strUserName;
	CStdString m_strPassword;
	CStdString m_strFileName;
	CStdString m_strProtocol;
	CStdString m_strFileType;
	CStdString m_strOptions;
	CStdString m_strProtocolOptions;
	CUrlOptions m_options;
	CUrlOptions m_protocolOptions;
};

