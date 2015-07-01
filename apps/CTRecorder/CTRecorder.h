#ifndef _CTRECORDER_H_
#define _CTRECORDER_H_

#include "AmAudioFile.h"
//#include "AmPlaylist.h"
#include "AmSession.h"

#include <regex.h>


class CTRecorderFactory: public AmSessionFactory
{
public:
	// Static variables

#if 0
	static vector<string>		s_Tags;
#endif

	static regex_t				x_rec_re;	

	static string				s_RecPath;
	static string				s_RecExt;
	static const string			s_RecTagsExt;
	static const string			s_RecReadyExt;
	static unsigned				s_RecMaxTime;


	CTRecorderFactory(const string& _app_name);
	virtual ~CTRecorderFactory();

	virtual int onLoad();

	virtual AmSession* onInvite(const AmSipRequest& req, const string& app_name, const map<string,string>& app_params);


private:
	// Private static variables

#if 0
	static const char * s_ctags[];
#endif

	static const string s_DefaultRecPath;
	static const string s_DefaultRecExt;

#if 0
	static const string s_X_REC_EXTRA;
	static const unsigned s_X_REC_EXTRA_NUM;
#endif
};

class CTRecorderDialog : public AmSession
{
public:
	CTRecorderDialog();
	virtual ~CTRecorderDialog();


	virtual void onSipRequest(const AmSipRequest& req);

	virtual void onInvite(const AmSipRequest& req);	

	virtual void onSessionStart();

	virtual void onBye(const AmSipRequest& req);

	virtual void process(AmEvent* event);

protected:
	/**
		Read tags from request headers
	*/
	bool getTags(const AmSipRequest& req, string& errstr);

	/**
		Manage end of recording (called upon receiving/sending bye)
	*/
	void endOfRecording();

	/**
		Save recording and associated tags
	*/
	bool saveAll();
	
	/**
		Copy the temporary recording to the destination file

		NOTE: The temporary file must be open
	*/
	bool saveRecordingFile();

	/**
		Remove the recording file
	*/

	bool removeRecordingFile();

	/**
		Save tags into the destination file
	*/
	bool saveTagsFile();

	/**
		Remove the tags file
	*/
	bool removeTagsFile();

	/**
		Save the ready file
	*/
	bool saveReadyFile();


private:
	//AmPlaylist		m_PlayList;

	AmAudioFile				m_RecFile;

	string					m_RecFilePath;
	string					m_RecTagsFilePath;
	string					m_RecReadyFilePath;

	string					m_RecorderAddr;
	string					m_Start;		// Recording begin timestamp (STAT_LOG format)
	unsigned				m_Duration;		// Recording duration in ms

	map<string, string>		m_Tags;
};

#endif
