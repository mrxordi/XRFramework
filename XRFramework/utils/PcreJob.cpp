#include "stdafxf.h"
#include "PcreJob.h"
#include "XRCommon/log/Log.h"
#include "XRCommon/utils/StringUtils.h"
#include "XRFramework/Base.h"
#include "JobManager.h"3
#include "Pcre.h"

// static variables
int CPcreJob::m_iUtf8Supported = -1;
int CPcreJob::m_iJitSupported = -1;


CPcreJob::CPcreJob(std::string pattern, MODE study, bool caseless, bool utf8) : reCompiled(nullptr), pcreExtra(nullptr), m_creatorCallback(nullptr)
{
	const char* modechar[] = { "NO STUDY", "STUDY", "JITUSED" };
	LOGINFO("Creating regex job with pattern: %s (%s%s)", pattern.c_str(), modechar[study], caseless ? " CASELESS" : "", utf8 ? " USING UTF-8" : "");

	InitValues(caseless, utf8);
	CompileRegexp(pattern, study);
}

CPcreJob::~CPcreJob()
{
	Cleanup();
}

void CPcreJob::Cleanup()
{
	if (reCompiled)
	{
		pcre_free(reCompiled);
		reCompiled = NULL;
	}

	if (pcreExtra)
	{
		pcre_free_study(pcreExtra);
		pcreExtra = NULL;
	}

	if (m_jitStack)
	{
		pcre_jit_stack_free(m_jitStack);
		m_jitStack = NULL;
	}
	m_creatorCallback = NULL;
}

bool CPcreJob::DoWork()
{
	bool success = 0;
	for (int i = 0; i < m_data.size(); i++) 
	{
		if (ShouldCancel(i + 1, m_data.size()))
			return false;
			
		int ret = FullMatch(m_data[i].data, m_data[i].vectorSize, m_data[i].matches, m_data[i].maxNumberOfCharsToTest, m_data[i].dataoffset);
		success = (ret>0);
		if (ret > 0)
			m_creatorCallback->OnMatchFound(m_data[i], i+1, this);
	}
	m_bRunning = false;
	return success;
}

const char * CPcreJob::GetType() const
{
	return "PCRE";
}

bool CPcreJob::CompileRegexp(std::string pattern, MODE study)
{
	if (pattern.empty())
		return false;

	m_jitCompiled		= false;
	m_bMatched			= false;
	m_iMatchCount		= 0;
	const char* errStr;
	int errorOffset		= 0;
	m_pattern = pattern;

	Cleanup();
	LOGINFO("Compiling regex with pattern: %s", pattern.c_str());

	
	// First, the regex string must be compiled.
	reCompiled = pcre_compile(pattern.c_str(), m_iOptions, &errStr, &errorOffset, NULL);

	/* OPTIONS (second argument) (||'ed together) can be:
	PCRE_ANCHORED       -- Like adding ^ at start of pattern.
	PCRE_CASELESS       -- Like m//i
	PCRE_DOLLAR_ENDONLY -- Make $ match end of string regardless of \n's
	No Perl equivalent.
	PCRE_DOTALL         -- Makes . match newlins too.  Like m//s
	PCRE_EXTENDED       -- Like m//x
	PCRE_EXTRA          --
	PCRE_MULTILINE      -- Like m//m
	PCRE_UNGREEDY       -- Set quantifiers to be ungreedy.  Individual quantifiers
	may be set to be greedy if they are followed by "?".
	PCRE_UTF8           -- Work with UTF8 strings.
	*/

	// pcre_compile returns NULL on error, and sets pcreErrorOffset & pcreErrorStr
	if (reCompiled == NULL) {
		m_pattern.clear();
		LOGERR("PCRE: %s. Compilation failed at offset %d in expression '%s'", errStr, errorOffset, pattern.c_str());
		return false;
	} /* end if */

	// Optimize the regex

	if (study) 
	{
		const bool jitCompile = (study == JITSTUDY) && IsJitSupported();
		const int studyOptions = jitCompile ? PCRE_STUDY_JIT_COMPILE : 0;

		pcreExtra = pcre_study(reCompiled, studyOptions, &errStr);
		if (errStr != nullptr)
		{
			LOGERR("PCRE error \"%s\" while studying expression", errStr);
			if (pcreExtra != nullptr)
			{
				pcre_free_study(pcreExtra);
				pcreExtra = nullptr;
			}
		}
		else if (jitCompile)
		{
			int jitPresent = 0;
			m_jitCompiled = (pcre_fullinfo(reCompiled, pcreExtra, PCRE_INFO_JIT, &jitPresent) == 0) && jitPresent == 1;
		}
	}
	return true;
}

void CPcreJob::LogBuildInfo()
{
	char* OutputStrings[] = { "UTF-8 support: ", "\"newline\" default character: ", 
		"bytes used for internal linkage in compiled regular expressions: ", 
		"POSIX MALLOC THRESHOLD: ",
		"default limit for the  number  of  internal  matching  function calls: ",
		"CONFIG STACKRECURSE: ",
		"Unicode character properties : ",
		"MATCH LIMIT RECURSION: ",
		"default line ending sequence: ",
		"Just-in-time compiling: ",
		"UTF-16 support: ",
		"JIT compiler is configured: ",
		"UTF-32 support: "
		};

	int i = 0;
	int result = PCRE_ERROR_BADOPTION;
	int intIutput = 0;
	char* charOutput = nullptr;

	LOGINFO("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	LOGINFO("+ PCRE UTF-8 Library build info, varsion: %s", pcre_version());

		while (i < 13)
		{

			if (i == PCRE_CONFIG_JITTARGET)
				result = pcre_config(i, &charOutput);
			else
				result = pcre_config(i, &intIutput);

			if (result == PCRE_ERROR_BADOPTION) {
				i++;
				continue;
			}

			switch (i)
			{
			case PCRE_CONFIG_JITTARGET:
				if (charOutput) {
					LOGINFO("%s : %s", OutputStrings[i], charOutput);
				}
				break;
			case PCRE_CONFIG_NEWLINE:
			{
				std::string output = "";
				switch (intIutput) {
				case 10:
					output = "LF";
					break;
				case 13:
					output = "CR";
					break;
				case 3338:
					output = "CRLF";
					break;
				case -2:
					output = "ANYCRLF";
					break;
				case -1:
					output = "ANY";
					break;
				case 21:
					output = "LF";
					break;
				default:
					LOGINFO("%s : %i", OutputStrings[i], intIutput);
				}

				if (output != "")
				{
					LOGINFO("%s : %s", OutputStrings[i], output.c_str());
				}
				break;
			}
			case PCRE_CONFIG_BSR:
				LOGINFO("%s : %s", OutputStrings[i], intIutput ? "Unicode line ending sequence" : "CR, LF, or CRLF");
				break;
			case PCRE_CONFIG_LINK_SIZE:
				LOGINFO("%s : %i", OutputStrings[i], intIutput);
				break;
			case PCRE_CONFIG_POSIX_MALLOC_THRESHOLD:
				LOGINFO("%s : %i", OutputStrings[i], intIutput);
				break;
			case  PCRE_CONFIG_MATCH_LIMIT:
				LOGINFO("%s : %i", OutputStrings[i], intIutput);
				break;
			case PCRE_CONFIG_MATCH_LIMIT_RECURSION:
				LOGINFO("%s : %i", OutputStrings[i], intIutput);
				break;
			case  PCRE_CONFIG_STACKRECURSE:
				LOGINFO("%s : %s", OutputStrings[i], intIutput ? "implemented by recursive function calls that use the stack to remember their state." : "use blocks of data on the heap instead of recursive function calls.")
					break;
			default:
				if (intIutput)
				{
					LOGINFO("%s %s", OutputStrings[i], "enabled.");
				}
				else
				{
					LOGERR("%s %s", OutputStrings[i], "disabled.");
				}
			}
			i++;
		}
	

	LOGINFO("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
}

bool CPcreJob::IsUtf8Supported()
{
	if (m_iUtf8Supported == -1)
	{
		if (pcre_config(PCRE_CONFIG_UTF8, &m_iUtf8Supported) != 0)
			m_iUtf8Supported = 0;

		if (m_iUtf8Supported && (pcre_config(PCRE_CONFIG_UNICODE_PROPERTIES, &m_iUtf8Supported) != 0))
			m_iUtf8Supported = 0;
	}

	if (!m_iUtf8Supported)
		LOGERR("Unicode properties are not enabled in PCRE lib, support for national symbols may be limited!");

	return m_iUtf8Supported == 1;
}

bool CPcreJob::IsJitSupported()
{
	if (m_iJitSupported == -1)
	{
		if (pcre_config(PCRE_CONFIG_JIT, &m_iJitSupported) != 0)
			m_iJitSupported = 0;
	}


	return m_iJitSupported == 1;
}

void CPcreJob::InitValues(bool caseless, bool utf8)
{
	m_iOptions = PCRE_DOTALL | PCRE_NEWLINE_ANY;
	if (caseless)
		m_iOptions |= PCRE_CASELESS;
	if (utf8 && IsUtf8Supported()) {
		m_iOptions |= PCRE_UTF8;
		m_iOptions |= PCRE_UCP;
		m_bUseUtf = true;
	}
	else
		m_bUseUtf = false;
	m_jitCompiled = false;
	m_bMatched = false;
	m_iMatchCount = 0;
	m_jitStack = nullptr;
}

int CPcreJob::FullMatch(std::string text, int matchVectorSize, std::vector<MATCH> &vector, int maxNumberOfCharsToTest /*= -1*/, int startOffset /*= 0*/)
{
	int textlen = text.length();
	std::string subject;
	int oVectCount = (matchVectorSize+1)*3;
	int* ioVector = new int[oVectCount];

	if (!reCompiled) {
		LOGERR("PCRE: Called before compilation");
		return -1;
	}
	if (text.empty()) {
		LOGERR("PCRE: Called without a string to match");
		return -1;
	}

	if (startOffset > text.length())
	{
		LOGERR("PCRE: startoffset is beyond end of string to match");
		return -1;
	}

	if (m_jitCompiled && !m_jitStack)
	{
		m_jitStack = pcre_jit_stack_alloc(32 * 1024, 512 * 1024);
		if (m_jitStack == NULL)
			LOGERR("Cant't allocate address space for JIT stack");

		pcre_assign_jit_stack(pcreExtra, NULL, m_jitStack);
	}
	for (int a = 0;; a++)
	{

		if (maxNumberOfCharsToTest >= 0)
			textlen = std::min<size_t>(textlen, startOffset + maxNumberOfCharsToTest);

		subject.assign(text.substr(startOffset, textlen - startOffset));

		int rc = (m_jitCompiled && !m_jitStack) ? pcre_jit_exec(reCompiled, pcreExtra, subject.c_str(), subject.length(), 0, 0, ioVector, oVectCount, m_jitStack)
			: pcre_exec(reCompiled, pcreExtra, subject.c_str(), subject.length(), 0, 0, ioVector, oVectCount);

		if (rc < -1 || (startOffset == 0 && rc == -1)) { // Something bad happened..
			switch (rc) {
			case PCRE_ERROR_NOMATCH: LOGERR("String did not match the pattern\n");        break;
			case PCRE_ERROR_NULL: LOGERR("Something was null\n");                      break;
			case PCRE_ERROR_BADOPTION: LOGERR("A bad option was passed\n");                 break;
			case PCRE_ERROR_BADMAGIC: LOGERR("Magic number bad (compiled re corrupt?)\n"); break;
			case PCRE_ERROR_UNKNOWN_NODE: LOGERR("Something kooky in the compiled re\n");      break;
			case PCRE_ERROR_NOMEMORY: LOGERR("Ran out of memory\n");                       break;
			default: LOGERR("Unknown error\n");                           break;
			} /* end switch */
			return -1;
		}
		else {
			if (rc == -1)
			{
				LOGINFO("We have %i matches.", a);
				SAFE_DELETE_ARRAY(ioVector);
				return a;
			}
			LOGINFO("Result: We have a match!\n");

			const char *psubStrMatchStr = nullptr;
			MATCH match;
			int longestoffset = 0;
			for (int k = 0; k < rc; k++) {
				match.push_back(GROUPENTRY(startOffset + ioVector[k * 2], startOffset + ioVector[k * 2 + 1]));
#ifdef _DEBUG
				pcre_get_substring(subject.c_str(), ioVector, rc, k, &(psubStrMatchStr));
				LOGINFO("Match(%2d/%2d): (%2d,%2d): '%s'\n", k, rc - 1, startOffset + ioVector[k * 2], startOffset + ioVector[k * 2 + 1], psubStrMatchStr);
#endif // DEBUG
				longestoffset = std::max<int>(longestoffset, ioVector[k * 2 + 1]);
			} /* end for */

			startOffset += longestoffset;
			vector.push_back(match);
			// Free up the substring
			pcre_free_substring(psubStrMatchStr);
		}
	}
	SAFE_DELETE_ARRAY(ioVector);
}

unsigned int CPcreJob::Run(CPcre* callback)
{
	if (m_creatorCallback || m_bRunning)
		return -1;
	m_creatorCallback = callback;
	m_bRunning = true;

	return CJobManager::GetInstance().AddJob(this, m_creatorCallback, CJob::PRIORITY_NORMAL);
}
