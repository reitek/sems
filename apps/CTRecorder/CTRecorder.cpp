#include "CTRecorder.h"
#include "CTRecMsgUtils.h"

#include <ctime>
#include <iomanip>
#include <iosfwd>
#include <sstream>

using std::ostringstream;
using std::setfill;
using std::setw;


#define TIMERID_MAX_RECTIME		1

#define MOD_NAME	"CTRecorder"

EXPORT_SESSION_FACTORY(CTRecorderFactory,MOD_NAME);

// Global variables


#if 0
template<typename T, size_t N>
T * end(T (&ra)[N]) {
    return ra + N;
}
#endif

// Static functions

static bool remove_file(const string& path)
{
	if (unlink(path.c_str()) == -1)
	{
		if (errno != ENOENT)
		{
			ERROR("Could not remove %s: %d (%s)\n", path.c_str(), errno, strerror(errno));
		}

		return false;
	}

	return true;
}

static bool safe_makedir(const string& path, string* errstr = 0)
{
	// First, check if the directory already exists
	struct stat statbuf;

	int status = stat(path.c_str(), &statbuf);
	if (status == 0)
	{
		if (!S_ISDIR(statbuf.st_mode))
		{
			if (errstr == NULL)
			{
				ERROR("%s is not a directory", path.c_str());
			}
			else
			{
				*errstr = path + " is not a directory";
			}
			return false;
		}

		if (!statbuf.st_mode & S_IRWXU)
		{
			if (errstr == NULL)
			{
				ERROR("%s must be RWX by the owner", path.c_str());
			}
			else
			{
				*errstr = path + " must be RWX by the owner";
			}
			return false;
		}

		// !!! CHECK: Check number of already existent entries into the directory ???

		return true;
	}

	if (errno != ENOENT)
	{
		if (errstr == NULL)
		{
			ERROR("Could not stat directory %s: %d (%s)", path.c_str(), errno, strerror(errno));
		}
		else
		{
			ostringstream ostr;
			ostr << "Could not stat directory " << path << ": " << errno << " (" << strerror(errno) << ")";
			*errstr = ostr.str();
		}

		return false;
	}

	// In this case, try to create all the path components

	string _path;

	size_t current;
	size_t next = -1;
	do
	{
		current = next + 1;
		next = path.find_first_of("/", current);

		string substr = path.substr(current, next-current);
		//DBG("substr: %s\n", substr.c_str());

		if (!substr.empty())
		{
			_path += "/" + substr;

			//DBG("_path: %s", _path.c_str());

			status = mkdir(_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (status && (errno != EEXIST))
			{
				if (errstr == NULL)
				{
					ERROR("Could not create directory %s path component %s: %d (%s)\n", path.c_str(), _path.c_str(), errno, strerror(errno));
				}
				else
				{
					ostringstream ostr;
					ostr << "Could not create directory " << path << " path component " << _path << ": " << errno << " (" << strerror(errno) << ")";
					*errstr = ostr.str();
				}
				
				return false;
			}
		}
	}
	while (next != string::npos);

#if 0
	_path += "/_test_dir_";
	status = mkdir(_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status && (errno != EEXIST))
	{
		ERROR("Write permission check failed on directory %s: %d (%s)\n", path.c_str(), errno, strerror(errno));
		return false;
	}

	rmdir(_path.c_str());
#endif

	return true;
}

static string make_date(string* errstr = 0)
{
	time_t rawtime;
	struct tm timeinfo;

	if (time(&rawtime) == -1)
	{
		if (errstr == NULL)
		{
			ERROR("time failed: %d (%s)\n", errno, strerror(errno));
		}
		else
		{
			ostringstream ostr;
			ostr << "time failed: " << errno << " (" << strerror(errno) << ")";
			*errstr = ostr.str();
		}

		return string();
	}

	if (localtime_r(&rawtime, &timeinfo) == NULL)
	{
		if (errstr == NULL)
		{
			ERROR("localtime_r failed: %d (%s)\n", errno, strerror(errno));
		}
		else
		{
			ostringstream ostr;
			ostr << "localtime_r failed: " << errno << " (" << strerror(errno) << ")";
			*errstr = ostr.str();
		}

		return string();
	}

	ostringstream strStream;
	strStream << (1900+timeinfo.tm_year) << "/" << setfill('0') << setw(2) << (1+timeinfo.tm_mon) << "/" << setfill('0') << setw(2) << (timeinfo.tm_mday);

	return strStream.str();
}

static string make_timestamp(string * errstr = 0)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1)
	{
		if (errstr == NULL)
		{
			ERROR("gettimeofday failed: %d (%s)\n", errno, strerror(errno));
		}
		else
		{
			ostringstream ostr;
			ostr << "gettimeofday failed: " << errno << " (" << strerror(errno) << ")";
			*errstr = ostr.str();
		}

		return string();
	}

	//DBG("tv.tv_sec: %ld\n", tv.tv_sec);
	//DBG("tv.tv_usec: %ld\n", tv.tv_usec);

	struct tm timeinfo;

	// Local time

	if (localtime_r(&tv.tv_sec, &timeinfo) == NULL)
	{
		if (errstr == NULL)
		{
			ERROR("localtime_r failed: %d (%s)\n", errno, strerror(errno));
		}
		else
		{
			ostringstream ostr;
			ostr << "localtime_r failed: " << errno << " (" << strerror(errno) << ")";
			*errstr = ostr.str();
		}	

		return string();
	}

#if 0
	// UTC time

	if (gmtime_r(&tv.tv_sec, &timeinfo) == NULL)
	{
		if (errstr == NULL)
		{
			ERROR("gmtime_r failed: %d (%s)\n", errno, strerror(errno));
		}
		else
		{
			ostringstream ostr;
			ostr << "gmtime_r failed: " << errno << " (" << strerror(errno) << ")";
			*errstr = ostr.str();
		}	

		return string();
	}
#endif

	//const char * format = "%z %Y-%m-%d %H:%M:%S";	// STAT_LOG format
	const char * format = "%Y-%m-%d %H:%M:%S";	// MySQL DATETIME format

	char dstbuf[40];
	memset(&dstbuf, 0, sizeof(dstbuf));
	if (strftime(dstbuf, sizeof(dstbuf), format, &timeinfo) == 0)
	{
		if (errstr == NULL)
		{
			ERROR("strftime failed: %d (%s)\n", errno, strerror(errno));
		}
		else
		{
			ostringstream ostr;
			ostr << "strftime failed: " << errno << " (" << strerror(errno) << ")";
			*errstr = ostr.str();
		}

		return string();	
	}

	ostringstream strStream;
	strStream << dstbuf << "." << setfill('0') << setw(3) << (tv.tv_usec / 1000);

	return strStream.str();
}

// CTRecorderFactory

// Private static variables

#if 0
const char * CTRecorderFactory::s_ctags[] = {
												"X-REC_EXTENSION",
												"X-REC_AGENT",
												"X-REC_VUUID",
												"X-REC_REMOTE",
											};

const string CTRecorderFactory::s_X_REC_EXTRA = "X-REC_EXTRA";
const unsigned CTRecorderFactory::s_X_REC_EXTRA_NUM = 30;
#endif

// Public static variables

const string CTRecorderFactory::s_DefaultRecPath = "/var/opt/reitek/CTRecorder/audio/";
const string CTRecorderFactory::s_DefaultRecExt = "wav";

#if 0
vector<string> CTRecorderFactory::s_Tags(CTRecorderFactory::s_ctags, end(CTRecorderFactory::s_ctags));
#endif

regex_t CTRecorderFactory::x_rec_re;

string CTRecorderFactory::s_RecPath;
string CTRecorderFactory::s_RecExt;
const string CTRecorderFactory::s_RecTagsExt = "tags";
const string CTRecorderFactory::s_RecReadyExt = "ready";
unsigned CTRecorderFactory::s_RecMaxTime = 0;

CTRecorderFactory::CTRecorderFactory(const string& _app_name)
  : AmSessionFactory(_app_name)
{
#if 0
	s_Tags.reserve(s_Tags.size() + CTRecorderFactory::s_X_REC_EXTRA_NUM);

	// Add X-REC_EXTRA headers to known tag headers
	for (unsigned idx = 0, max = CTRecorderFactory::s_X_REC_EXTRA_NUM; idx < max; idx++)
	{
		ostringstream strStream;
		strStream << CTRecorderFactory::s_X_REC_EXTRA << idx;

		s_Tags.push_back(strStream.str());
	}
#endif
}

CTRecorderFactory::~CTRecorderFactory()
{
}

int CTRecorderFactory::onLoad()
{
	DBG("CTRecorderFactory::onLoad\n");

	// !!! FIXME: Hardwired!!!
	if (regcomp(&CTRecorderFactory::x_rec_re, "^X-REC_", REG_EXTENDED|REG_NOSUB))
	{
		ERROR("Could not compile ^X-REC_ regex\n");

		return -1;
	}

	// !!! TODO: Check available codecs

	AmConfigReader cfg;
	if (cfg.loadFile(AmConfig::ModConfigPath + string(MOD_NAME ".conf")))
	{
		ERROR("Could not open config file");

		return -1;
	}

	s_RecPath = CTRecStringTrim(cfg.getParameter("rec_path", CTRecorderFactory::s_DefaultRecPath));
	if (s_RecPath.empty())
	{
		ERROR("Empty rec_path");
		return -1;
	}

	if (s_RecPath[s_RecPath.length()-1] != '/' )
		s_RecPath += "/";

	if (!safe_makedir(s_RecPath))
	{
		return -1;
	}

	INFO("rec_path: %s\n", s_RecPath.c_str());

	s_RecExt = CTRecStringTrim(cfg.getParameter("rec_ext", s_DefaultRecExt));
	if (s_RecExt.empty())
	{
		ERROR("Empty rec_ext");
		return -1;
	}
	else
	{
		bool fileExtOK = false;

		if (s_RecExt == "wav")
			fileExtOK = true;
#if 0
		else if (s_RecExt == "mp3")
			fileExtOK = true;
#endif

		if (!fileExtOK)
		{
			ERROR("Unsupported file extension %s: only wav is supported", s_RecExt.c_str());
			return -1;
		}
	}

	INFO("rec_ext: %s\n", s_RecExt.c_str());	

	s_RecMaxTime = cfg.getParameterInt("rec_maxtime", 0);

	INFO("rec_maxtime: %u\n", s_RecMaxTime);

	return 0;
}

AmSession* CTRecorderFactory::onInvite(const AmSipRequest& req, const string& app_name, const map<string,string>& app_params)
{
	DBG("CTRecorderFactory::onInvite\n");

	DBG("req: %s\n", req.print().c_str());
	DBG("app_name: %s\n", app_name.c_str());

	return new CTRecorderDialog();
}

// CTRecorderDialog

CTRecorderDialog::CTRecorderDialog()
// : m_PlayList(this)
{
}

CTRecorderDialog::~CTRecorderDialog()
{
	//m_PlayList.flush();
}

void CTRecorderDialog::onSipRequest(const AmSipRequest& req)
{
	if (req.method != "OPTIONS")
	{
		AmSession::onSipRequest(req);
		return;
	}

	// Reply to OPTIONS keep-alives with 200 OK

	DBG("OPTIONS keep-alive received: reply with 200 OK");

	dlg.reply(req,200,"OK");
}

void CTRecorderDialog::onInvite(const AmSipRequest& req)
{
	DBG("CTRecorderDialog::onInvite - dlg status = %s", dlg.getStatusStr());

	// NOTE: This makes sure to avoid doing it upon a re-INVITE
	if (dlg.getStatus() == AmSipDialog::Trying)
	{
		INFO("Concurrent sessions: %d\n", AmSession::getSessionNum());

		const int oif = dlg.getOutboundIf();
		m_RecorderAddr = AmConfig::Ifs[oif].LocalSIPIP + ":" + int2str(AmConfig::Ifs[oif].LocalSIPPort);

		DBG("RecorderAddr: %s\n", m_RecorderAddr.c_str());

		string errstr;

		// Extract tags from the request

		if (!getTags(req, errstr))
			// !!! CHECK: Send a client error instead of a server one?
			throw string(errstr);

		string today = make_date(&errstr);
		if (today.empty())
			throw string(errstr);

		//DBG("today: %s\n", today.c_str());

		m_Start = make_timestamp(&errstr);
		if (m_Start.empty())
			throw string(errstr);

		DBG("Start: %s\n", m_Start.c_str());

		if (!safe_makedir(CTRecorderFactory::s_RecPath + today + "/", &errstr))
			throw string(errstr);

		// !!! TODO: the output format should be overridable from request headers	

		m_RecFilePath = (CTRecorderFactory::s_RecPath + today + "/") + getCallID() + "." + CTRecorderFactory::s_RecExt;
		m_RecTagsFilePath = (CTRecorderFactory::s_RecPath + today + "/") + getCallID() + "." + CTRecorderFactory::s_RecTagsExt;
		m_RecReadyFilePath = (CTRecorderFactory::s_RecPath + today + "/") + getCallID() + "." + CTRecorderFactory::s_RecReadyExt;

		DBG("m_RecFilePath: %s\n", m_RecFilePath.c_str());
		DBG("m_RecTagsFilePath: %s\n", m_RecTagsFilePath.c_str());
		DBG("m_RecReadyFilePath: %s\n", m_RecReadyFilePath.c_str());
	}

	AmSession::onInvite(req);
}

void CTRecorderDialog::onSessionStart()
{
	// disable DTMF detection - don't use DTMF here
	setDtmfDetectionEnabled(false);	

	if (m_RecFile.open(m_RecFilePath, AmAudioFile::Write, true))
		throw string("Couldn't open temporary file for ") + m_RecFilePath;

	//m_PlayList.addToPlaylist(new AmPlaylistItem(NULL, &m_RecFile));

	//setInOut(&m_PlayList, &m_PlayList);

	setInput(&m_RecFile);
	setMute(true);

	if (CTRecorderFactory::s_RecMaxTime > 0)
	{
		setTimer(TIMERID_MAX_RECTIME, CTRecorderFactory::s_RecMaxTime);

		DBG("Started %us max recording timer", CTRecorderFactory::s_RecMaxTime);
	}

	AmSession::onSessionStart();
}

void CTRecorderDialog::onBye(const AmSipRequest& req)
{
	dlg.reply(req, 200, "OK");

#if 0
	setInput(NULL);

	// Make sure to properly finish writing the file (e.g.: wav headers) and to keep the file open
	m_RecFile.on_close();

	saveAll();

	m_RecFile.close();
#endif

	endOfRecording();

	setStopped();
}

void CTRecorderDialog::process(AmEvent* event)
{
	AmPluginEvent* plugin_event = dynamic_cast<AmPluginEvent*>(event);
	if (plugin_event && plugin_event->name == "timer_timeout" && plugin_event->data.get(0).asInt() == TIMERID_MAX_RECTIME)
	{
		DBG("max recording timer expired");

		dlg.bye();

		endOfRecording();

		setStopped();

		return;
	}

	return AmSession::process(event);
}

/**
	Read tags from request headers
*/
bool CTRecorderDialog::getTags(const AmSipRequest& req, string& errstr)
{
#if 0
	vector<string>::const_iterator iter = CTRecorderFactory::s_Tags.begin();
	while (iter != CTRecorderFactory::s_Tags.end())
	{
		DBG("cfg tag: %s\n", (*iter).c_str());

		string hdrval = getHeader(req.hdrs, (*iter).c_str(), true);
		if (!hdrval.empty())
			m_Tags[(*iter).c_str()] = hdrval;

		++iter;
	}
#endif

	DBG("req.hdrs: %s\n", req.hdrs.c_str());

	CTRecMIMEInfo mimeinfo;
	mimeinfo.ReadFrom(req.hdrs);

	m_Tags = mimeinfo.FindByRegex(CTRecorderFactory::x_rec_re);

	map<string,string>::const_iterator mapiter = m_Tags.begin();
	while (mapiter != m_Tags.end())
	{
		DBG("got tag %s: %s\n", (*mapiter).first.c_str(), (*mapiter).second.c_str());

		++mapiter;
	}

	return true;
}

/**
	Manage end of recording (called upon receiving/sending bye)
*/
void CTRecorderDialog::endOfRecording()
{
	setInput(NULL);

	// Make sure to properly finish writing the file (e.g.: wav headers) and to keep the file open
	m_RecFile.on_close();

	saveAll();

	m_RecFile.close();
}

/**
	Save recording and associated tags
*/
bool CTRecorderDialog::saveAll()
{
	if (!saveRecordingFile())
	{
		removeRecordingFile();

		return false;
	}

	if (!saveTagsFile() || !saveReadyFile())
	{
		removeRecordingFile();
		removeTagsFile();

		return false;
	}

	return true;
}
	
/**
	Copy the temporary recording to the destination file

	NOTE: The temporary file must be open
*/
bool CTRecorderDialog::saveRecordingFile()
{
	FILE * infp = m_RecFile.getfp();
	if (!infp)
	{
		ERROR("Temporary file for %s already closed", m_RecFilePath.c_str());
		return false;
	}

	//unsigned int rec_size = m_RecFile.getDataSize();
	//DBG("recorded data size: %i\n",rec_size);

#if 0
	const unsigned rec_length = m_RecFile.getLength();
	char rec_len_str[10];
	snprintf(rec_len_str, sizeof(rec_len_str), "%.2f", float(rec_length)/1000.0);
	const string rec_len_s = rec_len_str;
#endif

	m_Duration = m_RecFile.getLength();

	char rec_len_str[10];
	snprintf(rec_len_str, sizeof(rec_len_str), "%.2f", float(m_Duration)/1000.0);
	//const string rec_len_s = rec_len_str;

	DBG("Duration: %u ms (%s sec)\n", m_Duration, rec_len_str);

	FILE * outfp = fopen(m_RecFilePath.c_str(), "wb");
	if (!outfp)
	{
		ERROR("Could not create %s: %d (%s)\n", m_RecFilePath.c_str(), errno, strerror(errno));
		return false;
	}

	//m_RecFile.rewind();
	rewind(infp);

	char buf[1024];
	size_t nread;
	while (!feof(infp))
	{
		nread = fread(buf, 1, 1024, infp);
		if (fwrite(buf, 1, nread, outfp) != nread)
			// !!! TODO: Error checking !!!
			break;
	}

	fclose(outfp);

	DBG("%s successfully saved", m_RecFilePath.c_str());

	return true;
}

/**
	Remove the recording file
*/

bool CTRecorderDialog::removeRecordingFile()
{
	return remove_file(m_RecTagsFilePath);
}

/**
	Save tags into the destination file
*/
bool CTRecorderDialog::saveTagsFile()
{
	FILE * outfp = fopen(m_RecTagsFilePath.c_str(), "wb");
	if (!outfp)
	{
		ERROR("Could not create %s: %d (%s)\n", m_RecTagsFilePath.c_str(), errno, strerror(errno));
		return false;
	}

	string str;
	ostringstream ostr;

	str	= "RecorderAddr: " + m_RecorderAddr + "\n";
	if (fputs(str.c_str(), outfp) == -1)
	{
		ERROR("Could not write RecorderAddr %s into %s: %d (%s)\n", m_RecorderAddr.c_str(), m_RecTagsFilePath.c_str(), errno, strerror(errno));

		fclose(outfp);

		return false;
	}

	str	= "Call-ID: " + getCallID() + "\n";
	if (fputs(str.c_str(), outfp) == -1)
	{
		ERROR("Could not write Call-ID %s into %s: %d (%s)\n", getCallID().c_str(), m_RecTagsFilePath.c_str(), errno, strerror(errno));

		fclose(outfp);

		return false;
	}

	str	= "Start: " + m_Start + "\n";
	if (fputs(str.c_str(), outfp) == -1)
	{
		ERROR("Could not write Start %s into %s: %d (%s)\n", m_Start.c_str(), m_RecTagsFilePath.c_str(), errno, strerror(errno));

		fclose(outfp);

		return false;
	}

	ostr << "Duration: " << m_Duration << "\n";
	if (fputs(ostr.str().c_str(), outfp) == -1)
	{
		ERROR("Could not write Duration %u into %s: %d (%s)\n", m_Duration, m_RecTagsFilePath.c_str(), errno, strerror(errno));

		fclose(outfp);

		return false;
	}

	str = "FullPath: " + m_RecFilePath + "\n";
	if (fputs(str.c_str(), outfp) == -1)
	{
		ERROR("Could not write FullPath %s into %s: %d (%s)\n", m_RecFilePath.c_str(), m_RecTagsFilePath.c_str(), errno, strerror(errno));

		fclose(outfp);

		return false;
	}

#if 0
	bool err = false;
#endif

	map<string,string>::const_iterator mapiter = m_Tags.begin();
	while (mapiter != m_Tags.end())
	{
		str = (*mapiter).first + ": " + (*mapiter).second + "\n";

		if (fputs(str.c_str(), outfp) == -1)
		{
			ERROR("Could not write %s %s into %s: %d (%s)\n", (*mapiter).first.c_str(), (*mapiter).second.c_str(), m_RecTagsFilePath.c_str(), errno, strerror(errno));

			fclose(outfp);

			return false;
		}		

#if 0		
		if (fputs((*mapiter).first.c_str(), outfp) == -1)
		{
			err = true;
			break;
		}

		if (fputs(": ", outfp) == -1)
		{
			err = true;
			break;
		}

		if (fputs((*mapiter).second.c_str(), outfp) == -1)
		{
			err = true;
			break;
		}

		if (fputs("\n", outfp) == -1)
		{
			err = true;
			break;
		}
#endif

		++mapiter;
	}

#if 0
	if (err)
	{
		ERROR("Could not write %s: %d (%s)\n", m_RecTagsFilePath.c_str(), errno, strerror(errno));	
	}
#endif

	fclose(outfp);

#if 0
	if (err)
	{
		return false;
	}
#endif

	DBG("%s successfully saved", m_RecTagsFilePath.c_str());	

	return true;
}

/**
	Remove the tags file
*/
bool CTRecorderDialog::removeTagsFile()
{
	return remove_file(m_RecFilePath);
}

/**
	Save the ready file
*/
bool CTRecorderDialog::saveReadyFile()
{
	FILE * outfp = fopen(m_RecReadyFilePath.c_str(), "wb");
	if (!outfp)
	{
		ERROR("Could not create %s: %d (%s)\n", m_RecReadyFilePath.c_str(), errno, strerror(errno));
		return false;
	}

	fclose(outfp);

	DBG("%s successfully saved", m_RecReadyFilePath.c_str());

	return true;
}
