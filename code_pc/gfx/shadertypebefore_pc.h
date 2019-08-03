#ifndef _SHADERTYPE_H_
#define _SHADERTYPE_H_

#define uniform

#define SHADER_GETX X()
#define SHADER_GETY Y()
#define SHADER_GETZ Z()
#define SHADER_GETW W()

#define CONST_REFERENCE(type) const type&
#define SHADER_STATICCONST(type,name,value) static const type name = value;


inline float saturate(float f)
{
	return ntstd::Clamp(f,0.0f,1.0f);
}
inline float pow(float f,float p)
{
	return ::pow(f,p);
}
inline float lerp(float a, float b, float s)
{
	return SimpleFunction::Lerp(a,b,s);
}
typedef Pixel2 int2;
typedef Pixel3 int3;
typedef Pixel4 int4;
typedef CVector float3;
typedef CVector float4;
typedef CMatrix float4x4;



#endif // end of _SHADERTYPE_H_