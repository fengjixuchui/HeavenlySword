#ifndef _BITMASK_H_
#define _BITMASK_H_



//--------------------------------------------------
//!
//!	basic bitmask class.
//!	if you want to store several bool in an integer
//!	The reaon I wrote another bitmask class is to for typesafety:
//! You have to give the State enum type (see an example)
//!
//--------------------------------------------------

template<typename State, typename Type = uint8_t>
class BitMask
{
public:
	typedef Type Unsigned;
	Unsigned m_uMask;
	//! constructor
	explicit inline BitMask(): m_uMask(GetNone()) {}
	explicit inline BitMask(Unsigned uMask): m_uMask(uMask) {}
	inline void Reset() {m_uMask=GetNone();}
	inline void Set(State s) {m_uMask |= s;}
	inline void Unset(State s) {m_uMask &= (s ^ GetAll());}
	inline void Set(State s,bool value) { if(value) Set(s); else Unset(s);}
	inline void Toggle(State s) {m_uMask ^= s;}
	inline bool CheckFlag(State s) const {return (m_uMask&s)==s;}
	inline bool AllOfThem(State s) const {return (m_uMask&s)==s;}
	inline bool AtLeastOne(State s) const {return (m_uMask&s)!=GetNone();}
	inline bool operator[] (State s) const  {return AtLeastOne(s);}
	static inline Unsigned GetAll() {return static_cast<Unsigned>(-1);}
	static inline Unsigned GetNone() {return static_cast<Unsigned>(0);}
	//static inline Unsigned GetFlag(int i) { return static_cast<Unsigned>(1<<i);}
}; // end of class BitMask

#define BITFLAG(i) (1<<i)



#endif // end of _BITMASK_H_
