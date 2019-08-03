#ifndef _NUMERICS_H_
#define _NUMERICS_H_



// math class (redirection to proper function according to type
template<class T>
class Numerics {};

// int math class
template<>
class Numerics<char>
{
public:
	static inline char zero() {return static_cast<char>(0);}
};

// int math class
template<>
class Numerics<int>
{
public:
	static inline int abs(int iV) {return ::abs(iV);}
	static inline int zero() {return 0;}
	static inline int one() {return 1;}
};

// float math class
template<>
class Numerics<float>
{
public:
	static inline float abs(float fV) {return ::fabs(fV);}
	static inline float sqrt(float fV) {return ::sqrt(fV);}
	static inline float zero() {return 0.0f;}
	static inline float one() {return 1.0f;}
};

// float math class
template<>
class Numerics<CVector>
{
public:
	static inline CVector zero() {return CVector(0.0f);}
	static inline CVector one() {return CVector(1.0f);}
};



#endif // end of _NUMERICS_H_
