//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpXml parser : main class

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.

	@see		FpXmlParser.cpp
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_XML_PARSER_H
#define FP_XML_PARSER_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpXml/FpXmlStringAsciiTo.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class FpXmlParser : public FwNonCopyable
{
public:
	// Enumerations
	enum 
	{
		kMaxDepth=32							///< Completely arbitrary value to avoid wasting too much space with the stack, can easily be changed!
	};
	enum NodeType
	{
		// All other node types are ignored since these node have alredy been handled by the 
		// preprocessor.
		kNodeTypeNone				=0x0000,	
		kNodeTypeCommand			=0x0001,	///< <?...>
		kNodeTypeElement			=0x0002,	///< <...>
		kNodeTypeElementEnd			=0x0004,	///< </...>
		kNodeTypeText				=0x0008,	///< >...<
		kNodeTypeUnsupported		=0x0010,	///< Unsupported node type (critical!)
		kNodeTypeBaseMask			=0x00FF,	///< Mask for base type

		kNodeTypeModifierEmpty		=0x1000,	///< Empty element
		kNodeTypeModifierSimple		=0x2000,	///< Simple element (one text child)
		kNodeTypeModifierComplex	=0x4000,	///< Complex element (more than one child or element child)
		kNodeTypeModifierMask		=0xFF00,	///< Mask for element modifier flags

		kNodeTypeAny				=0xFFFFFFFF
	};
	enum Encoding
	{
		kEncodingUnknown			=0,			///< Unknown encoding (ascii assumed)
		kEncodingAscii,							///< Ascii (<=127)
		kEncodingUtf8,							///< UTF8
		kEncodingUtf16Le,						///< UTF-16-LE little endian (NOT supported)
		kEncodingUtf16Be,						///< UTF-16-BE big-endian (NOT supported)
		kEncodingUtf32Le,						///< UTF-32-LE little endian (NOT supported)
		kEncodingUtf32Be,						///< UTF-32-BE big-endian (NOT supported)
		kEncodingShiftJis,						///< Shift_JIS (NOT supported)
		kEncodingOtherUnsupported				///< Any other unsupported encoding scheme
	};

	// Typedefs
	typedef void (*ErrorFunction) (const char* pFormatString, ...);

	// Constants
	static const char kMarkerTagStart=0;			///< Internal marker used in preprocessed data. MUST be zero to allow text elements to end as c strings 
	static const char kMarkerTagEndEmpty=1;			///< Internal marker used in preprocessed data.
	static const char kMarkerTagEndSimple=2;		///< Internal marker used in preprocessed data.
	static const char kMarkerTagEndComplex=3;		///< Internal marker used in preprocessed data.
	static const char kMarkerAttributesStart=4;		///< Internal marker used in preprocessed data.
	static const char kMarkerAttributeSeparator=0;	///< Internal marker used in preprocessed data. MUST be zero
	static const char kMarkerLast=4;				///< Last internal marker 

	// Construction & Destruction
	FpXmlParser(void);
   ~FpXmlParser(void){};

	///@name Init 
   	//@{
	bool		SetXmlData(char* const pBuffer, const size_t sizeBuffer);
	//@}

	///@name Navigation (main position)
	//@{
	bool		ResetPos(void);

	bool 		FindNextElement(const char* const pElementName=NULL);
	bool 		FindElement(const char* const pElementName);
	bool		FindNextNode(const NodeType mask=FpXmlParser::kNodeTypeAny);
	NodeType	GetNodeBaseType(void) const;

	const char*	GetTagName(void) const;
	const char*	GetDataText(void) const;

	bool		FindAttribute(const char* const pAttributeName=NULL);
	const char* GetAttributeName(void) const;
	const char* GetAttributeValue(void) const;
	//@}

	///@name Navigation (child position)
	//@{
	bool		ResetChildPos(void);

	bool		FindNextChildElement(const char* const pElementName=NULL);
	bool		FindChildElement(const char* const pElementName);
	bool		FindNextChildNode(const NodeType mask=FpXmlParser::kNodeTypeAny);
	NodeType	GetChildNodeBaseType(void) const;

	const char* GetChildTagName(void) const;
	const char* GetChildDataText(void) const;

	bool		FindChildAttribute(const char* const pAttributeName=NULL);
	const char* GetChildAttributeName(void) const;
	const char* GetChildAttributeValue(void) const;	

	///@name Navigation between child and main position
	//@{
	bool 		IntoElement(void);
	bool 		OutOfElement(void);

	inline int 	GetMainDepth(void)	{return m_depthMain;}
	inline int 	GetChildDepth(void)	{return m_depthChild;}
	//@}

	///@name Read helpers (current main element data) 
	//@{
	float		GetDataFloat(float defaultValueIfFailed=0.0f);
	bool		GetDataBool(bool defaultValueIfFailed=false);
	u64 		GetDataU64(u64 defaultValueIfFailed=0ULL);
	s64 		GetDataS64(s64 defaultValueIfFailed=0LL);
	u32 		GetDataU32(u32 defaultValueIfFailed=0);
	s32 		GetDataS32(s32 defaultValueIfFailed=0);
	u16 		GetDataU16(u16 defaultValueIfFailed=0);
	s16 		GetDataS16(s16 defaultValueIfFailed=0);
	u8 			GetDataU8(u8 defaultValueIfFailed=0);
	s8 			GetDataS8(s8 defaultValueIfFailed=0);

	int 		ReadData(float* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(bool* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(u64* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(s64* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(u32* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(s32* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(u16* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(s16* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(u8* pDest, int numToRead=1, const char separator=0);
	int 		ReadData(s8* pDest, int numToRead=1, const char separator=0);
	//@}

	///@name Read helpers (current child element data) 
	//@{	
	float		GetChildDataFloat(float defaultValueIfFailed=0.0f);
	bool		GetChildDataBool(bool defaultValueIfFailed=false);
	u64 		GetChildDataU64(u64 defaultValueIfFailed=0ULL);
	s64 		GetChildDataS64(s64 defaultValueIfFailed=0LL);
	u32 		GetChildDataU32(u32 defaultValueIfFailed=0);
	s32 		GetChildDataS32(s32 defaultValueIfFailed=0);
	u16 		GetChildDataU16(u16 defaultValueIfFailed=0);
	s16 		GetChildDataS16(s16 defaultValueIfFailed=0);
	u8 			GetChildDataU8(u8 defaultValueIfFailed=0);
	s8 			GetChildDataS8(s8 defaultValueIfFailed=0);

	int 		ReadChildData(float* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(bool* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(u64* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(s64* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(u32* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(s32* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(u16* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(s16* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(u8* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildData(s8* pDest, int numToRead=1, const char separator=0);
	//@} 

	///@name Read helpers (current main attribute data) 
	//@{
	float		GetAttributeValueFloat(float defaultValueIfFailed=0.0f);
	bool		GetAttributeValueBool(bool defaultValueIfFailed=false);
	u64 		GetAttributeValueU64(u64 defaultValueIfFailed=0ULL);
	s64 		GetAttributeValueS64(s64 defaultValueIfFailed=0LL);
	u32 		GetAttributeValueU32(u32 defaultValueIfFailed=0);
	s32 		GetAttributeValueS32(s32 defaultValueIfFailed=0);
	u16 		GetAttributeValueU16(u16 defaultValueIfFailed=0);
	s16 		GetAttributeValueS16(s16 defaultValueIfFailed=0);
	u8 			GetAttributeValueU8(u8 defaultValueIfFailed=0);
	s8 			GetAttributeValueS8(s8 defaultValueIfFailed=0);

	int 		ReadAttributeValue(float* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(bool* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(u64* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(s64* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(u32* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(s32* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(u16* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(s16* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(u8* pDest, int numToRead=1, const char separator=0);
	int 		ReadAttributeValue(s8* pDest, int numToRead=1, const char separator=0);
	//@}

	///@name Read helpers (current child attribute data) 
	//@{
	float		GetChildAttributeValueFloat(float defaultValueIfFailed=0.0f);
	bool		GetChildAttributeValueBool(bool defaultValueIfFailed=false);
	u64 		GetChildAttributeValueU64(u64 defaultValueIfFailed=0ULL);
	s64 		GetChildAttributeValueS64(s64 defaultValueIfFailed=0LL);
	u32 		GetChildAttributeValueU32(u32 defaultValueIfFailed=0);
	s32 		GetChildAttributeValueS32(s32 defaultValueIfFailed=0);
	u16 		GetChildAttributeValueU16(u16 defaultValueIfFailed=0);
	s16 		GetChildAttributeValueS16(s16 defaultValueIfFailed=0);
	u8 			GetChildAttributeValueU8(u8 defaultValueIfFailed=0);
	s8 			GetChildAttributeValueS8(s8 defaultValueIfFailed=0);

	int 		ReadChildAttributeValue(float* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(bool* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(u64* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(s64* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(u32* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(s32* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(u16* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(s16* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(u8* pDest, int numToRead=1, const char separator=0);
	int 		ReadChildAttributeValue(s8* pDest, int numToRead=1, const char separator=0);
	//@}

	///@name Misc
	//@{
	inline FpXmlParser::Encoding	GetEncoding(void) {return m_encoding;}
	static bool						IsSupportedEncoding(const FpXmlParser::Encoding encoding);
	static const char*				GetEncodingName(const FpXmlParser::Encoding encoding);
	static void 					DefaultErrorFunction(const char* pFormat, ...);
	static void 					SetErrorFunction(ErrorFunction pFunction);
	static ErrorFunction			Error;
	//@}
private:
	// Attributes
	char* const m_pBuffer;		///< Data buffer 
	char* const m_pBufferEnd;	///< End of data buffer(after preprocessing)
	class Context : public FwNonCopyable
	{
	public:
		///@name Init
		//@{
		void				Reset(void);
		//@}

		///@name Operations
		//@{
		inline NodeType 	GetNodeBaseType(void) const;
		inline NodeType 	GetNodeType(void) const;
		inline void 		DePatchTagName(void);
		inline void 		RePatchTagName(void);
		bool 				EnterNode(char* const pData, char* const pBufferEnd);
		bool 				LeaveNode(void);
		bool 				FindAttribute(const char* const pAttributeName=NULL);
		//@}

		///@name Accessors
		//@{
		inline const char* 	GetCurAttributeName(void) const {return m_pCurAttributeName;}
		inline const char* 	GetCurAttributeValue(void) const {return m_pCurAttributeValue;}
		inline const char* 	GetTagName(void) const {return m_pTagName;}
		inline const char* 	GetDataText(void) const {return m_pData;}
		inline char*		GetNextNode(void) const {return m_pNextNode;}
		inline char*		GetNextSibling(void) const {return m_pNextSibling;}
		inline void 		SetNextNode(char* pNext) {m_pNextNode=pNext;}
		inline void 		SetNextSibling(char* pNext) {m_pNextSibling=pNext;}
		inline char* 		GetNodeStart(void) {return m_pStart;}
		inline char* 		GetFirstChild(void) {return m_pFirstChild;}
		//@}

	private:
		// Attributes
		NodeType			m_typeNode;				///< Type of the node
		char*				m_pStart;				///< Start of tag
		const char* 		m_pBufferEnd;			///< We need to save this here because we don't know the FpXmlParser
		const char* 		m_pTagName;				///< Tag name (warning, only valid during the lifetime of this context)
		const char* 		m_pData;				///< Node data (only set if text node, or to text data for a simple element such @code<tag>(->)text</tag>@endcode
		const char* 		m_pFirstAttribute;		///< First attribute (set to NULL if there is no attribute)
		const char* 		m_pCurAttributeName;	///< Current attribute name (NULL if none), always associated m_pCurAttributeValue
		const char* 		m_pCurAttributeValue;	///< Current attribute data (NULL if none), always associated m_pCurAttributeName
		char* 				m_pFirstChild;			///< First child (set if the node is a non empty element)
		char* 				m_pNextNode;			///< Next node (=end of this node)
		char* 				m_pNextSibling;			///< Next sibling (if it can easily be known; only a helper)
		// Temporary patch done on EnterNode() and removed at LeaveNode() to have c-string compliant tag name
		char*				m_pPatched;
		char				m_patched;
	}						m_contextStack[kMaxDepth];
	int 					m_depthMain;			///< Depth of the current main position in the context stack (-1 if no current main position)
	int 					m_depthChild;			///< Depth of the child position in the context stack (should be always m_main+1 if defined, -1 if no child)					
	Encoding				m_encoding;				///< Encoding (asciim utf8, whatever)

	///@name Init
	//@{
	void Reset(void);
	//@}

	///@name Internal Operations
	//@{
	Context* 			GetMainContext(const int offset=0);
	Context* 			GetChildContext(const int offset=0);
	const Context* 		GetMainContext(const int offset=0) const;
	const Context* 		GetChildContext(const int offset=0) const;
	char*				FindNodeInternal(const NodeType typeMask, const int depthStart, const int depthFind, 
										 char* const pStartFind, char* const pEndFind, bool ignoreFirst);
	bool 				FindNode(const NodeType typeMask, const char* const pNameToFind, bool useChild, 
								 bool nextOnly);
	void 				DePatchTagNames(void);
	void 				RePatchTagNames(void);
	bool 				SetEncoding(const char* pEncodingName);
	//@}

	///@name Internal helpers
	//@{
	static const char*	FindNextAttributeStart(const char* const pCurrent, const char* const pBufferEnd); 
	static char* 		FindNextTagEnd(char* const pCurrent, char* const pBufferEnd);
	static char* 		FindNextTagStart(char* const pCurrent, char* const pBufferEnd);
	static char* 		FindNextMarker(char* const pCurrent,char* const pBufferEnd); 
	static NodeType		GetNodeBaseType(const char* const pNode, 
 										const char* const pBufferEnd);
	//@}
};

//--------------------------------------------------------------------------------------------------

#endif	//FP_XML_PARSER_H
