//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file luaremotedebugmessages.cpp                                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _GOLD_MASTER

///////////////////////////////
// Required Includes           
///////////////////////////////

enum RDBGMSGTYPE
{
	SETBP = 1,
	REMBP,
	STEPOVER,
	STEPINTO,
	STEPOUT,
	GO,
	INFORM_LINE,
	INFORM_VAR,
	INFORM_STACK,
	RELOADSCRIPT,
	INFORM_OUTPUT,
	REQUESTGLOBAL,
	EXECUTESCRIPT
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg
//! Abstract Base Class for the lua debugging messages passed between the server and the
//! debugging client.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg
{
public:
	LuaDbgMsg() {;}
	virtual ~LuaDbgMsg() {;}

	virtual int Length() const = 0;
	virtual void Serialise(char* buf, int iLen) const = 0;
	virtual void Deserialise(char* buf, int iLen) = 0;
	virtual void Despatch() const = 0;

#ifdef _LUA_EDITOR
	static void SetSciTE(class SciTEBase* pSciTE) {m_pSciTE = pSciTE;}
	static SciTEBase* m_pSciTE;
#endif

	static LuaDbgMsg* Decode(char* buf, int iLen);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_SetBreakPoint
//! Request a breakpoint to be set on a given line in a script.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_SetBreakPoint : public LuaDbgMsg
{
public:
	LuaDbgMsg_SetBreakPoint() {m_acFile[0] = 0; m_iLine = -1;}
	LuaDbgMsg_SetBreakPoint(const char* pcFile, int iLine) {strcpy(m_acFile, pcFile); m_iLine = iLine;}
	~LuaDbgMsg_SetBreakPoint() {;}

	virtual int Length() const {return strlen(m_acFile) + sizeof(int) + 2;}
	static RDBGMSGTYPE GetType() {return SETBP;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

private:
	char  m_acFile[256];
	int   m_iLine;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_RemoveBreakPoint
//! Request that a specific breakpoint is removed. (Line -1 indicates all breakpoints.)
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_RemoveBreakPoint : public LuaDbgMsg
{
public:
	LuaDbgMsg_RemoveBreakPoint() {m_acFile[0] = 0; m_iLine = -1;}
	LuaDbgMsg_RemoveBreakPoint(const char* pcFile, int iLine) {strcpy(m_acFile, pcFile); m_iLine = iLine;}
	~LuaDbgMsg_RemoveBreakPoint() {;}

	virtual int Length() const {return strlen(m_acFile) + sizeof(int) + 2;}
	static RDBGMSGTYPE GetType() {return REMBP;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

private:
	char  m_acFile[256];
	int   m_iLine;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_InformCurrentLine
//! Inform the debugging client that we've breaked at a particular line.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_InformCurrentLine : public LuaDbgMsg
{
public:
	LuaDbgMsg_InformCurrentLine() {m_acFile[0] = 0; m_iLine = -1;}
	LuaDbgMsg_InformCurrentLine(const char* pcFile, int iLine)
	{
		// Trim the path
		//int i;
		//for(i = strlen(pcFile); i > 1 && pcFile[i-1] != '\\' && pcFile[i-1] != '/' && pcFile[i-1] != '@'; i--);

		strcpy(m_acFile, pcFile);//+i);
		m_iLine = iLine;
	}

	virtual int Length() const {return strlen(m_acFile) + sizeof(int) + 2;}
	static RDBGMSGTYPE GetType() {return INFORM_LINE;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

private:
	char  m_acFile[256];
	int m_iLine;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_StepOver
//! Request to step over the next line of script.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_StepOver : public LuaDbgMsg
{
public:
	LuaDbgMsg_StepOver() {;}

	virtual int Length() const {return 1;}
	static RDBGMSGTYPE GetType() {return STEPOVER;}
	virtual void Serialise(char* buf, int) const {buf[0] = (char)GetType();}
	virtual void Deserialise(char*, int) {;}
	virtual void Despatch() const;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_StepInto
//! Request to step into a script function.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_StepInto : public LuaDbgMsg
{
public:
	LuaDbgMsg_StepInto() {;}

	virtual int Length() const {return 1;}
	static RDBGMSGTYPE GetType() {return STEPINTO;}
	virtual void Serialise(char* buf, int) const {buf[0] = (char)GetType();}
	virtual void Deserialise(char*, int) {;}
	virtual void Despatch() const;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_StepOut
//! Request to step out of the current script function.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_StepOut : public LuaDbgMsg
{
public:
	LuaDbgMsg_StepOut() {;}

	virtual int Length() const {return 1;}
	static RDBGMSGTYPE GetType() {return STEPOUT;}
	virtual void Serialise(char* buf, int) const {buf[0] = (char)GetType();}
	virtual void Deserialise(char*, int) {;}
	virtual void Despatch() const;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_Go
//! Resume normal execution of a script.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_Go : public LuaDbgMsg
{
public:
	LuaDbgMsg_Go() {;}

	virtual int Length() const {return 1;}
	static RDBGMSGTYPE GetType() {return GO;}
	virtual void Serialise(char* buf, int) const {buf[0] = (char)GetType();}
	virtual void Deserialise(char*, int) {;}
	virtual void Despatch() const;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_InformVariable
//! Inform the debug client of the status of a script variable.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_InformVariable : public LuaDbgMsg
{
public:
	LuaDbgMsg_InformVariable() {m_acName[0] = 0; m_acType[0] = 0; m_acValue[0] =0; m_bGlobal = false; m_iParent = -1;}
	LuaDbgMsg_InformVariable(const char* pcName, const char* pcType, const char* pcValue, int iID, int iParent, bool bGlobal) 
	{strcpy(m_acName, pcName); strcpy(m_acType, pcType); strcpy(m_acValue, pcValue); m_iID = iID; m_iParent = iParent; m_bGlobal = bGlobal;}

	virtual int Length() const {return strlen(m_acName) + strlen(m_acType) + strlen(m_acValue) + sizeof(bool) + sizeof(int)*2 + 4;}
	static RDBGMSGTYPE GetType() {return INFORM_VAR;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

public:
	const char* GetVarName()   const {return m_acName;}
	const char* GetVarType()   const {return m_acType;}
	const char* GetVarValue()  const {return m_acValue;}
	bool        IsVarGlobal()  const {return m_bGlobal;}
	int         GetVarID()     const {return m_iID;}
	int         GetVarParent() const {return m_iParent;}

private:
	char  m_acName[256];
	char  m_acType[256];
	char  m_acValue[256];
	bool  m_bGlobal;
	int   m_iID;
	int   m_iParent;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_InformCallstack
//! Inform the debug client about the callstack.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_InformCallstack : public LuaDbgMsg
{
public:
	LuaDbgMsg_InformCallstack() {m_acFile[0] = m_acFunc[0] = 0; m_iLine = -1;}
	LuaDbgMsg_InformCallstack(const char* pcFunc, const char* pcFile, int iLine) {strcpy(m_acFunc, pcFunc); strcpy(m_acFile, pcFile); m_iLine = iLine;}

	virtual int Length() const {return strlen(m_acFunc) + strlen(m_acFile) + sizeof(int) + 3;}
	static RDBGMSGTYPE GetType() {return INFORM_STACK;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

public:
	const char* GetFunc() const {return m_acFunc;}
	const char* GetFile() const {return m_acFile;}
	int         GetLine() const {return m_iLine;}

private:
	char  m_acFunc[256];
	char  m_acFile[256];
	int   m_iLine;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_ReloadScript
//! Request that the script server should reload a specified script file.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_ReloadScript : public LuaDbgMsg
{
public:
	LuaDbgMsg_ReloadScript() {m_acFile[0] = 0;}
	LuaDbgMsg_ReloadScript(const char* pcFile) {strcpy(m_acFile, pcFile);}
	~LuaDbgMsg_ReloadScript() {;}

	virtual int Length() const {return strlen(m_acFile) + 2;}
	static RDBGMSGTYPE GetType() {return RELOADSCRIPT;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

private:
	char  m_acFile[256];
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_InformOutput
//! Output some information into the debug-output pane of the debug client.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_InformOutput : public LuaDbgMsg
{
public:
	LuaDbgMsg_InformOutput() {m_acMsg[0] = 0;}
	LuaDbgMsg_InformOutput(const char* pcMsg) {strcpy(m_acMsg, pcMsg);}

	virtual int Length() const {return strlen(m_acMsg) + 2;}
	static RDBGMSGTYPE GetType() {return INFORM_OUTPUT;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

private:
	char  m_acMsg[512];
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_RequestGlobal
//! Request that the debug server keep us informed of a particular global variable.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_RequestGlobal : public LuaDbgMsg
{
public:
	LuaDbgMsg_RequestGlobal() {m_acName[0] = 0;}
	LuaDbgMsg_RequestGlobal(const char* pcName) {strcpy(m_acName, pcName);}
	~LuaDbgMsg_RequestGlobal() {;}

	virtual int Length() const {return strlen(m_acName) + 2;}
	static RDBGMSGTYPE GetType() {return REQUESTGLOBAL;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

private:
	char  m_acName[256];
};

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaDbgMsg_ExecuteScript
//! Execute some script immediately.
//!                                                                                         
//------------------------------------------------------------------------------------------
class LuaDbgMsg_ExecuteScript : public LuaDbgMsg
{
public:
	LuaDbgMsg_ExecuteScript() {m_acScript[0] = 0;}
	LuaDbgMsg_ExecuteScript(const char* pcScript) {strcpy(m_acScript, pcScript);}
	~LuaDbgMsg_ExecuteScript() {;}

	virtual int Length() const {return strlen(m_acScript) + 2;}
	static RDBGMSGTYPE GetType() {return EXECUTESCRIPT;}
	virtual void Serialise(char* buf, int iLen) const;
	virtual void Deserialise(char* buf, int iLen);
	virtual void Despatch() const;

private:
	char  m_acScript[1000];
};

#endif
