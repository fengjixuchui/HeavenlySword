/***************************************************************************************************
*
*   $Header:: /game/maths.cpp 4     6/05/03 15:34 Dean                                             $
*
*	Static Maths Class
*
*	CHANGES		
*
*	15/05/2001	dean	Created
*	25/07/2005	wil		Checked in
*
***************************************************************************************************/

#if 0

/***************************************************************************************************
*
*	FUNCTION		Approximate Maths Library
*
*	DESCRIPTION		Nicked from Intel.. 
*
***************************************************************************************************/

#define AM_PI							(3.14159265358979323846f)

#define _PS_CONST(Name, Type, Val)	const _MM_ALIGN16 Type  _ps_##Name[4] = { Val, Val, Val, Val }

//			Name				Type		Value (propagated to 4 vector components)

_PS_CONST( am_0,				float,		0.0f );
_PS_CONST( am_1,				float,		1.0f );
_PS_CONST( am_m1,				float,		-1.0f );
_PS_CONST( am_0p5,				float,		0.5f );
_PS_CONST( am_1p5,				float,		1.5f );
_PS_CONST( am_pi,				float,		( float )AM_PI );
_PS_CONST( am_pi_o_2,			float,		( float )( AM_PI / 2.0) );
_PS_CONST( am_2_o_pi,			float,		( float )( 2.0 / AM_PI) );
_PS_CONST( am_pi_o_4,			float,		( float )( AM_PI / 4.0) );
_PS_CONST( am_4_o_pi,			float,		( float )( 4.0 / AM_PI) );
_PS_CONST( am_sign_mask,		__int32,	0x80000000 );
_PS_CONST( am_inv_sign_mask,	__int32,	~0x80000000 );
_PS_CONST( am_min_norm_pos,		__int32,	0x00800000 );
_PS_CONST( am_mant_mask,		__int32,	0x7f800000 );
_PS_CONST( am_inv_mant_mask,	__int32,	~0x7f800000 );


/***************************************************************************************************
*
*	FUNCTION		CMaths::Sinf
*
*	DESCRIPTION		Returns the sine of the input angle
*
*	NOTES			Range is -3.3732593e9 to +3.3732593e9
*
***************************************************************************************************/

_PS_CONST( sincos_p0,			float,		0.15707963267948963959e1f );
_PS_CONST( sincos_p1,			float,		-0.64596409750621907082e0f );
_PS_CONST( sincos_p2,			float,		0.7969262624561800806e-1f );
_PS_CONST( sincos_p3,			float,		-0.468175413106023168e-2f );

static const unsigned __int32 _sincos_masks[]		= { ( unsigned int )0x0, ( unsigned int )~0x0 };
static const unsigned __int32 _sincos_inv_masks[]	= { ( unsigned int )~0x0, ( unsigned int )0x0 };

float	CMaths::Sinf( float fRadians )
{
	float	fResult;
	float	fTemp;

	__asm
	{
		movss	xmm0, fRadians
		movss	[fTemp], xmm0
		movss	xmm1, _ps_am_inv_sign_mask
		mov		eax, [fTemp]
		mulss	xmm0, _ps_am_2_o_pi
		andps	xmm0, xmm1
		and		eax, 0x80000000

		cvttss2si	ecx, xmm0
		movss	xmm1, _ps_am_1
		mov		edx, ecx
		shl		edx, (31 - 1)
		cvtsi2ss	xmm2, ecx
		and		ecx, 0x1
		and		edx, 0x80000000

		subss	xmm0, xmm2
		movss	xmm6, _sincos_masks[ecx * 4]
		minss	xmm0, xmm1

		movss	xmm5, _ps_sincos_p3
		subss	xmm1, xmm0

		andps	xmm1, xmm6
		andnps	xmm6, xmm0
		orps	xmm1, xmm6
		movss	xmm4, _ps_sincos_p2
		movss	xmm0, xmm1

		mulss	xmm1, xmm1
		movss	xmm7, _ps_sincos_p1
		xor		eax, edx
		movss	xmm2, xmm1
		mulss	xmm1, xmm5
		movss	xmm5, _ps_sincos_p0
		mov		[fTemp], eax
		addss	xmm1, xmm4
		mulss	xmm1, xmm2
		movss	xmm3, [fTemp]
		addss	xmm1, xmm7
		mulss	xmm1, xmm2
		orps	xmm0, xmm3
		addss	xmm1, xmm5
		mulss	xmm0, xmm1
		movss	[fResult],xmm0
	}

	return fResult;
}


/***************************************************************************************************
*
*	FUNCTION		CMaths::Cosf
*
*	DESCRIPTION		Returns the cosine of the input angle
*
*	NOTES			Range is -3.3732593e9 to +3.3732593e9
*
***************************************************************************************************/

float	CMaths::Cosf( float fRadians )
{
	float	fResult;
	float	fTemp;

	__asm
	{
		movss	xmm0, fRadians
		movss	xmm1, _ps_am_inv_sign_mask
		andps	xmm0, xmm1
		addss	xmm0, _ps_am_pi_o_2
		mulss	xmm0, _ps_am_2_o_pi

		cvttss2si	ecx, xmm0
		movss	xmm5, _ps_am_1
		mov		edx, ecx
		shl		edx, (31 - 1)
		cvtsi2ss	xmm1, ecx
		and		edx, 0x80000000
		and		ecx, 0x1

		subss	xmm0, xmm1
		movss	xmm6, _sincos_masks[ecx * 4]
		minss	xmm0, xmm5

		movss	xmm1, _ps_sincos_p3
		subss	xmm5, xmm0

		andps	xmm5, xmm6
		movss	xmm7, _ps_sincos_p2
		andnps	xmm6, xmm0
		mov		[fTemp], edx
		orps	xmm5, xmm6
		movss	xmm0, xmm5

		mulss	xmm5, xmm5
		movss	xmm4, _ps_sincos_p1
		movss	xmm2, xmm5
		mulss	xmm5, xmm1
		movss	xmm1, _ps_sincos_p0
		addss	xmm5, xmm7
		mulss	xmm5, xmm2
		movss	xmm3, [fTemp]
		addss	xmm5, xmm4
		mulss	xmm5, xmm2
		orps	xmm0, xmm3
		addss	xmm5, xmm1
		mulss	xmm0, xmm5
		movss	[fResult], xmm0
	}

	return fResult;
}


/***************************************************************************************************
*
*	FUNCTION		CMaths::SinCos
*
*	DESCRIPTION		Returns both the sine and cosine of the input angle. This is faster than calling
*					sinf() and cosf() separately.
*
*	NOTES			Range is -3.3732593e9 to +3.3732593e9
*
***************************************************************************************************/

void	CMaths::SinCos( float fRadians, float& fSinStore, float& fCosStore )
{

//	fSinStore = CMaths::Sinf( fRadians );
//	fCosStore = CMaths::Cosf( fRadians );

#ifndef	_USE_SSE	
	fSinStore = fsinf( fRadians );
	fCosStore = fcosf( fRadians );
#else

	float	fTemp12;
	float	fTemp8;
	float	fTemp4;

	float	fSinResult;
	float	fCosResult;

	__asm
	{
		movss	xmm0, fRadians
		movss	[fTemp12], xmm0
		movss	xmm1, _ps_am_inv_sign_mask
		mov		eax, [fTemp12]
		mulss	xmm0, _ps_am_2_o_pi
		andps	xmm0, xmm1
		and		eax, 0x80000000

		cvttss2si	edx, xmm0
		mov		ecx, edx
		mov		[fTemp12], esi
		mov		esi, edx
		add		edx, 0x1	
		shl		ecx, (31 - 1)
		shl		edx, (31 - 1)

		movss	xmm4, _ps_am_1
		cvtsi2ss	xmm3, esi
		mov		[fTemp8], eax
		and		esi, 0x1

		subss	xmm0, xmm3
		movss	xmm3, _sincos_inv_masks[esi * 4]
		minss	xmm0, xmm4

		subss	xmm4, xmm0

		movss	xmm6, xmm4
		andps	xmm4, xmm3
		and		ecx, 0x80000000
		movss	xmm2, xmm3
		andnps	xmm3, xmm0
		and		edx, 0x80000000
		movss	xmm7, [fTemp8]
		andps	xmm0, xmm2
		mov		[fTemp8], ecx
		mov		[fTemp4], edx
		orps	xmm4, xmm3

		andnps	xmm2, xmm6
		orps	xmm0, xmm2

		movss	xmm2, [fTemp8]
		movss	xmm1, xmm0
		movss	xmm5, xmm4
		xorps	xmm7, xmm2
		movss	xmm3, _ps_sincos_p3
		mulss	xmm0, xmm0
		mulss	xmm4, xmm4
		movss	xmm2, xmm0
		movss	xmm6, xmm4
		orps	xmm1, xmm7
		movss	xmm7, _ps_sincos_p2
		mulss	xmm0, xmm3
		mulss	xmm4, xmm3
		movss	xmm3, _ps_sincos_p1
		addss	xmm0, xmm7
		addss	xmm4, xmm7
		movss	xmm7, _ps_sincos_p0
		mulss	xmm0, xmm2
		mulss	xmm4, xmm6
		addss	xmm0, xmm3
		addss	xmm4, xmm3
		movss	xmm3, [fTemp4]
		mulss	xmm0, xmm2
		mulss	xmm4, xmm6
		orps	xmm5, xmm3
		mov		esi,  [fTemp12]
		addss	xmm0, xmm7
		addss	xmm4, xmm7
		mulss	xmm0, xmm1
		mulss	xmm4, xmm5

		// Save away the results..

		movss	[fSinResult], xmm0
		movss	[fCosResult], xmm4
	}

	fSinStore = fSinResult;
	fCosStore = fCosResult;

#endif	//_USE_SSE
}


/***************************************************************************************************
*
*	FUNCTION		CMaths::Tanf
*
*	DESCRIPTION		Returns the tangent of the input angle
*
*	NOTES			Range is -FLT_MAX to +FLT_MAX
*
***************************************************************************************************/

_PS_CONST( tan_p0,		float,	-1.79565251976484877988e7f );
_PS_CONST( tan_p1,		float,	1.15351664838587416140e6f );
_PS_CONST( tan_p2,		float,	-1.30936939181383777646e4f );
	
_PS_CONST( tan_q0,		float,	-5.38695755929454629881e7f );
_PS_CONST( tan_q1,		float,	2.50083801823357915839e7f );
_PS_CONST( tan_q2,		float,	-1.32089234440210967447e6f );
_PS_CONST( tan_q3,		float,	1.36812963470692954678e4f );

_PS_CONST( tan_poleval,	float,	3.68935e19f );

float	CMaths::Tanf( float fRadians )
{
	float	fResult;
	float	fTemp8;
	float	fTemp4;

	__asm
	{
		movss	xmm0, fRadians
		movss	[fTemp8], xmm0
		mov		[fTemp4], esi
		movss	xmm1, _ps_am_inv_sign_mask
		mov		eax, [fTemp8]
		andps	xmm0, xmm1
		and		eax, 0x80000000
		movss	xmm1, xmm0
		mulss	xmm0, _ps_am_4_o_pi

		cvttss2si	edx, xmm0
		
		movss	xmm5, _ps_am_1

		mov		ecx, 0x1
		mov		esi, 0x7

		and		ecx, edx
		and		esi, edx
		add		edx, ecx
		add		esi, ecx
		mov		[fTemp8], eax

		cvtsi2ss	xmm0, edx
		xorps	xmm6, xmm6

		mulss	xmm0, _ps_am_pi_o_4
		test	esi, 0x2
		subss	xmm1, xmm0
		movss	xmm2, _ps_tan_p2
		minss	xmm1, xmm5
		movss	xmm3, _ps_tan_q3
		movss	xmm0, xmm1
		mulss	xmm1, xmm1
		movss	xmm7, [fTemp8]

		mulss	xmm2, xmm1
		addss	xmm3, xmm1
		addss	xmm2, _ps_tan_p1
		mulss	xmm3, xmm1
		mulss	xmm2, xmm1
		addss	xmm3, _ps_tan_q2
		addss	xmm2, _ps_tan_p0
		mulss	xmm3, xmm1
		mulss	xmm2, xmm1
		addss	xmm3, _ps_tan_q1
		xorps	xmm0, xmm7
		mulss	xmm3, xmm1
		mulss	xmm2, xmm0
		addss	xmm3, _ps_tan_q0

		rcpss	xmm4, xmm3
		mulss	xmm3, xmm4
		mov		esi, [fTemp4]
		mulss	xmm3, xmm4
		addss	xmm4, xmm4
		subss	xmm4, xmm3

		mulss	xmm2, xmm4
		jz		l_cont
		addss	xmm2, xmm0
		comiss	xmm6, xmm1

		rcpss	xmm4, xmm2
		movss	xmm0, _ps_am_sign_mask
		jz		l_pole
		mulss	xmm2, xmm4
		mulss	xmm2, xmm4
		addss	xmm4, xmm4
		subss	xmm4, xmm2
		xorps	xmm0, xmm4

		jmp		all_over

l_pole:
		movss	xmm1, _ps_tan_poleval
		movss	xmm3, xmm0
		andps	xmm0, xmm2
		orps	xmm0, xmm1

		xorps	xmm0, xmm3

		jmp		all_over

l_cont:
		addss	xmm0, xmm2

all_over:
		movss	[fResult], xmm0
	
	}

	return fResult;
}


/***************************************************************************************************
*
*	FUNCTION		CMaths::Atanf
*
*	DESCRIPTION		Returns the arc tangent of fX
*
*	NOTES			Range is -FLT_MAX to +FLT_MAX
*
***************************************************************************************************/

_PS_CONST( atan_t0,		float,	-0.91646118527267623468e-1f );
_PS_CONST( atan_t1,		float,	-0.13956945682312098640e1f );
_PS_CONST( atan_t2,		float,	-0.94393926122725531747e2f );
_PS_CONST( atan_t3,		float,	0.12888383034157279340e2f );

_PS_CONST( atan_s0,		float,	0.12797564625607904396e1f );
_PS_CONST( atan_s1,		float,	0.21972168858277355914e1f );
_PS_CONST( atan_s2,		float,	0.68193064729268275701e1f );
_PS_CONST( atan_s3,		float,	0.28205206687035841409e2f );

float	CMaths::Atanf( float fX )
{
	float	fResult;

	__asm
	{
		movss	xmm0, fX
		movss	xmm1, _ps_am_sign_mask
		rcpss	xmm4, xmm0
		orps	xmm1, xmm0
		movss	xmm6, xmm4
		comiss	xmm1, _ps_am_m1
		movss	xmm3, _ps_atan_t0
		jc		l_big  // 'c' is 'lt' for comiss

		movss	xmm2, xmm0
		mulss	xmm2, xmm2

		movss	xmm1, _ps_atan_s0
		addss	xmm1, xmm2

		movss	xmm7, _ps_atan_s1
		rcpss	xmm1, xmm1
		mulss	xmm1, xmm3
		movss	xmm3, _ps_atan_t1
		addss	xmm7, xmm2
		addss	xmm1, xmm7
			
		movss	xmm7, _ps_atan_s2
		rcpss	xmm1, xmm1
		mulss	xmm1, xmm3
		movss	xmm3, _ps_atan_t2
		addss	xmm7, xmm2
		addss	xmm1, xmm7

		movss	xmm7, _ps_atan_s3
		rcpss	xmm1, xmm1
		mulss	xmm1, xmm3
		movss	xmm3, _ps_atan_t3
		addss	xmm7, xmm2
		mulss	xmm0, xmm3
		addss	xmm1, xmm7

		rcpss	xmm1, xmm1
		mulss	xmm0, xmm1

		jmp		all_over

l_big:
		mulss	xmm6, xmm6

		movss	xmm5, _ps_atan_s0
		addss	xmm5, xmm6

		movss	xmm7, _ps_atan_s1
		rcpss	xmm5, xmm5
		mulss	xmm5, xmm3
		movss	xmm3, _ps_atan_t1
		addss	xmm7, xmm6
		addss	xmm5, xmm7

		movss	xmm7, _ps_atan_s2
		rcpss	xmm5, xmm5
		mulss	xmm5, xmm3
		movss	xmm3, _ps_atan_t2
		addss	xmm7, xmm6
		addss	xmm5, xmm7

		movss	xmm7, _ps_atan_s3
		rcpss	xmm5, xmm5
		mulss	xmm5, xmm3
		movss	xmm3, _ps_atan_t3
		addss	xmm7, xmm6
		movss	xmm2, _ps_am_sign_mask
		mulss	xmm4, xmm3
		addss	xmm5, xmm7

		movss	xmm7, _ps_am_pi_o_2
		rcpss	xmm5, xmm5
		mulss	xmm5, xmm4

		andps	xmm0, xmm2
		orps	xmm0, xmm7
		subss	xmm0, xmm5

all_over:
		movss	[fResult], xmm0
	}

	return fResult;
}     


/***************************************************************************************************
*
*	FUNCTION		CMaths::Expf
*
*	DESCRIPTION		Returns the exponent fo the input value
*
*	NOTES			Range is -FLT_MAX to +88.3762626647949
*
***************************************************************************************************/

_PS_CONST( exp_hi,		float,	88.3762626647949f );
_PS_CONST( exp_lo,		float,	-88.3762626647949f );

_PS_CONST( exp_rln2,	float,	1.4426950408889634073599f );

_PS_CONST( exp_p0,		float,	1.26177193074810590878e-4f );
_PS_CONST( exp_p1,		float,	3.02994407707441961300e-2f );

_PS_CONST( exp_q0,		float,	3.00198505138664455042e-6f );
_PS_CONST( exp_q1,		float,	2.52448340349684104192e-3f );
_PS_CONST( exp_q2,		float,	2.27265548208155028766e-1f );
_PS_CONST( exp_q3,		float,	2.00000000000000000009e0f );

_PS_CONST( exp_c1,		float,	6.93145751953125e-1f );
_PS_CONST( exp_c2,		float,	1.42860682030941723212e-6f );

float	CMaths::Expf( float fInput )
{
	float	fResult;
	float	fTemp;

	__asm
	{
		movss	xmm0, fInput
		minss	xmm0, _ps_exp_hi
		movss	xmm1, _ps_exp_rln2
		maxss	xmm0, _ps_exp_lo
		mulss	xmm1, xmm0
		movss	xmm7, _ps_am_0
		addss	xmm1, _ps_am_0p5
		xor		ecx, ecx

		mov		edx, 1
		comiss	xmm1, xmm7
		cvttss2si	eax, xmm1
		cmovc	ecx, edx  // 'c' is 'lt' for comiss
		sub		eax, ecx

		cvtsi2ss	xmm1, eax
		add		eax, 0x7f

		movss	xmm2, xmm1
		mulss	xmm1, _ps_exp_c1
		mulss	xmm2, _ps_exp_c2
		subss	xmm0, xmm1
		shl		eax, 23
		subss	xmm0, xmm2

		movss	xmm2, xmm0
		mov 	[fTemp], eax
		mulss	xmm2, xmm2

		movss	xmm6, _ps_exp_q0
		movss	xmm4, _ps_exp_p0

		mulss	xmm6, xmm2
		movss	xmm7, _ps_exp_q1
		mulss	xmm4, xmm2
		movss	xmm5, _ps_exp_p1

		addss	xmm6, xmm7
		addss	xmm4, xmm5

		movss	xmm7, _ps_exp_q2
		mulss	xmm6, xmm2
		mulss	xmm4, xmm2

		addss	xmm6, xmm7
		mulss	xmm4, xmm0

		movss	xmm7, _ps_exp_q3
		mulss	xmm6, xmm2
		addss	xmm4, xmm0
		addss	xmm6, xmm7
		movss	xmm0, [fTemp]

		subss	xmm6, xmm4
		rcpss	xmm6, xmm6  
		movss	xmm7, _ps_am_1
		mulss	xmm4, xmm6
		addss	xmm4, xmm4
		addss	xmm4, xmm7

		mulss	xmm0, xmm4
		movss	[fResult], xmm0
	}
}    


/***************************************************************************************************
*
*	FUNCTION		CMaths::Exp2f
*
*	DESCRIPTION		Returns the binary exponent of the input value
*
*	NOTES			Range is -FLT_MAX to +127.4999961853
*
***************************************************************************************************/

_PS_CONST( exp2_hi,	float,	127.4999961853f );
_PS_CONST( exp2_lo,	float,	-127.4999961853f );

_PS_CONST( exp2_p0,	float,	2.30933477057345225087e-2f );
_PS_CONST( exp2_p1,	float,	2.02020656693165307700e1f );
_PS_CONST( exp2_p2,	float,	1.51390680115615096133e3f );

_PS_CONST( exp2_q0,	float,	2.33184211722314911771e2f );
_PS_CONST( exp2_q1,	float,	4.36821166879210612817e3f );

float	CMaths::Exp2f( float fInput )
{
	float	fResult;
	float	fTemp;

	__asm
	{
		movss	xmm0, fInput
		minss	xmm0, _ps_exp2_hi
		movss	xmm5, _ps_am_0p5
		maxss	xmm0, _ps_exp2_lo
		xorps	xmm7, xmm7
		addss	xmm5, xmm0
		xor		ecx, ecx

		mov		edx, 1
		comiss	xmm5, xmm7
		cvttss2si	eax, xmm5
		cmovc	ecx, edx  // 'c' is 'lt' for comiss
		sub		eax, ecx

		cvtsi2ss	xmm5, eax
		add		eax, 0x7f

		subss	xmm0, xmm5

		movss	xmm2, xmm0
		mulss	xmm2, xmm2

		movss	xmm6, _ps_exp2_q0
		movss	xmm4, _ps_exp2_p0

		mulss	xmm6, xmm2
		movss	xmm7, _ps_exp2_q1
		mulss	xmm4, xmm2
		movss	xmm5, _ps_exp2_p1

		shl		eax, 23
		addss	xmm6, xmm7
		addss	xmm4, xmm5

		movss	xmm5, _ps_exp2_p2
		mulss	xmm4, xmm2

		addss	xmm4, xmm5

		mulss	xmm4, xmm0

		mov 	[fTemp], eax
		subss	xmm6, xmm4
		movss	xmm7, _ps_am_1
		rcpss	xmm6, xmm6  
		mulss	xmm4, xmm6
		movss	xmm0, [fTemp]
		addss	xmm4, xmm4
		addss	xmm4, xmm7

		mulss	xmm0, xmm4
		movss	[fResult], xmm0
	}

	return fResult;
}


/***************************************************************************************************
*
*	FUNCTION		CMaths::Logf
*
*	DESCRIPTION		Returns the logarithm of the input value
*
*	NOTES			Range is 1.17549e-038 to +FLT_MAX
*
***************************************************************************************************/

_PS_CONST( log_p0,	float,	-7.89580278884799154124e-1f );
_PS_CONST( log_p1,	float,	1.63866645699558079767e1f );
_PS_CONST( log_p2,	float,	-6.41409952958715622951e1f );

_PS_CONST( log_q0,	float,	-3.56722798256324312549e1f );
_PS_CONST( log_q1,	float,	3.12093766372244180303e2f );
_PS_CONST( log_q2,	float,	-7.69691943550460008604e2f );

_PS_CONST( log_c0,	float,	0.693147180559945f );

float	CMaths::Logf( float fInput )
{
	float	fResult;
	float	fTemp;

	__asm
	{
		movss	xmm0, fInput
		maxss	xmm0, _ps_am_min_norm_pos  // cut off denormalized stuff
		movss	xmm7, _ps_am_inv_mant_mask
		movss	xmm1, _ps_am_1
		movss	[fTemp], xmm0

		andps	xmm0, xmm7
		orps	xmm0, xmm1
		movss	xmm7, xmm0
		mov		edx, [fTemp]

		addss	xmm7, xmm1
		subss	xmm0, xmm1
		rcpss	xmm7, xmm7  
		mulss	xmm0, xmm7
		addss	xmm0, xmm0

		shr		edx, 23

		movss	xmm2, xmm0
		sub		edx, 0x7f
		mulss	xmm2, xmm2

		movss	xmm4, _ps_log_p0
		movss	xmm6, _ps_log_q0

		mulss	xmm4, xmm2
		movss	xmm5, _ps_log_p1
		mulss	xmm6, xmm2
		movss	xmm7, _ps_log_q1

		addss	xmm4, xmm5
		addss	xmm6, xmm7

		movss	xmm5, _ps_log_p2
		mulss	xmm4, xmm2
		cvtsi2ss	xmm1, edx
		movss	xmm7, _ps_log_q2
		mulss	xmm6, xmm2

		addss	xmm4, xmm5
		addss	xmm6, xmm7

		movss	xmm5, _ps_log_c0
		mulss	xmm4, xmm2
		rcpss	xmm6, xmm6  

		mulss	xmm4, xmm0
		mulss	xmm1, xmm5
		mulss	xmm4, xmm6

		addss	xmm0, xmm1
		addss	xmm0, xmm4
		movss	[fResult],xmm0
	}

	return fResult;
}


/***************************************************************************************************
*
*	FUNCTION		CMaths::Log2f
*
*	DESCRIPTION		Returns the binary logarithm of the input value
*
*	NOTES			Range is 1.17549e-038 to +FLT_MAX4
*
***************************************************************************************************/

_PS_CONST( log2_c0,	float,	1.44269504088896340735992f );

float	CMaths::Log2f( float fInput )
{
	float	fResult;
	float	fTemp;

	__asm
	{
		movss	xmm0, fInput
		maxss	xmm0, _ps_am_min_norm_pos  // cut off denormalized stuff
		movss	xmm7, _ps_am_inv_mant_mask
		movss	xmm1, _ps_am_1
		movss	[fTemp], xmm0

		andps	xmm0, xmm7
		orps	xmm0, xmm1
		movss	xmm7, xmm0
		mov		edx, [fTemp]

		addss	xmm7, xmm1
		subss	xmm0, xmm1
		rcpss	xmm7, xmm7  
		mulss	xmm0, xmm7
		addss	xmm0, xmm0

		shr		edx, 23

		movss	xmm4, _ps_log_p0
		movss	xmm6, _ps_log_q0

		movss	xmm2, xmm0
		sub		edx, 0x7f
		mulss	xmm2, xmm2

		mulss	xmm4, xmm2
		movss	xmm5, _ps_log_p1
		mulss	xmm6, xmm2
		movss	xmm7, _ps_log_q1

		addss	xmm4, xmm5
		addss	xmm6, xmm7

		movss	xmm5, _ps_log_p2
		mulss	xmm4, xmm2
		movss	xmm7, _ps_log_q2
		mulss	xmm6, xmm2

		addss	xmm4, xmm5
		addss	xmm6, xmm7

		movss	xmm5, _ps_log2_c0
		mulss	xmm4, xmm2
		rcpss	xmm6, xmm6  

		cvtsi2ss	xmm1, edx

		mulss	xmm6, xmm0
		mulss	xmm4, xmm6
		addss	xmm0, xmm4
		mulss	xmm0, xmm5

		addss	xmm0, xmm1
		movss	[fResult], xmm0
	}

	return fResult;
}


/***************************************************************************************************
*
*	FUNCTION		CMaths::Powf
*
*	DESCRIPTION		Returns fX raised to the power fY
*
*	NOTES			Range is (for fX )	:	1.17549e-038 to +FLT_MAX
*					Range is (for fY )	:	fY * log2f(fX) <= 127.4999961853
*
***************************************************************************************************/

float	CMaths::Powf( float fX, float fY )
{
	float	fResult;
	float	fTemp;

	__asm
	{
		movss	xmm0, fX
		movss	xmm1, fY
		xorps	xmm7, xmm7
		comiss	xmm7, xmm0
		movss	xmm7, _ps_am_inv_mant_mask
		maxss	xmm0, _ps_am_min_norm_pos  // cut off denormalized stuff
		jnc		l_zerobase
		movss	xmm3, _ps_am_1
		movss	[fTemp], xmm0

		andps	xmm0, xmm7
		orps	xmm0, xmm3
		movss	xmm7, xmm0

		addss	xmm7, xmm3
		subss	xmm0, xmm3
		mov		edx, [fTemp]
		rcpss	xmm7, xmm7  
		mulss	xmm0, xmm7
		addss	xmm0, xmm0

		shr		edx, 23

		movss	xmm4, _ps_log_p0
		movss	xmm6, _ps_log_q0

		sub		edx, 0x7f
		movss	xmm2, xmm0
		mulss	xmm2, xmm2

		mulss	xmm4, xmm2
		movss	xmm5, _ps_log_p1
		mulss	xmm6, xmm2
		cvtsi2ss	xmm3, edx
		movss	xmm7, _ps_log_q1

		addss	xmm4, xmm5
		mulss	xmm3, xmm1
		addss	xmm6, xmm7

		movss	xmm5, _ps_log_p2
		mulss	xmm4, xmm2
		movss	xmm7, _ps_log_q2
		mulss	xmm6, xmm2

		addss	xmm4, xmm5
		mulss	xmm1, _ps_log2_c0
		addss	xmm6, xmm7

		mulss	xmm4, xmm2
		rcpss	xmm6, xmm6  

		mulss	xmm6, xmm0
		mulss	xmm4, xmm6
		movss	xmm6, _ps_exp2_hi
		addss	xmm0, xmm4
		movss	xmm4, _ps_exp2_lo
		xorps	xmm7, xmm7
		movss	xmm5, _ps_am_0p5
		mulss	xmm0, xmm1

		addss	xmm0, xmm3
		xor		ecx, ecx

		minss	xmm0, xmm6
		mov		edx, 1
		maxss	xmm0, xmm4

		addss	xmm5, xmm0

		comiss	xmm5, xmm7
		cvttss2si	eax, xmm5
		cmovc	ecx, edx  // 'c' is 'lt' for comiss
		sub		eax, ecx

		cvtsi2ss	xmm5, eax
		add		eax, 0x7f

		subss	xmm0, xmm5

		movss	xmm2, xmm0
		mulss	xmm2, xmm2

		movss	xmm6, _ps_exp2_q0
		movss	xmm4, _ps_exp2_p0

		mulss	xmm6, xmm2
		movss	xmm7, _ps_exp2_q1
		mulss	xmm4, xmm2
		movss	xmm5, _ps_exp2_p1

		shl		eax, 23
		addss	xmm6, xmm7
		addss	xmm4, xmm5

		movss	xmm5, _ps_exp2_p2
		mulss	xmm4, xmm2

		addss	xmm4, xmm5

		mulss	xmm4, xmm0

		mov 	[fTemp], eax
		subss	xmm6, xmm4
		movss	xmm7, _ps_am_1
		rcpss	xmm6, xmm6  
		mulss	xmm4, xmm6
		movss	xmm0, [fTemp]
		addss	xmm4, xmm4
		addss	xmm4, xmm7

		mulss	xmm0, xmm4

		jmp		all_over

l_zerobase:
		xorps	xmm0, xmm0

all_over:
		movss	[fResult], xmm0
	}
	
	return fResult;
}

#endif // 0

/***************************************************************************************************
*
*	FUNCTION		CQuadraticSolver::SetCoefficients
*
*	DESCRIPTION		Sets the coefficients of the quadratic.
*
***************************************************************************************************/

void CQuadraticSolver::SetCoefficients( float fA, float fB, float fC )
{
	// set them
	m_fA = fA;
	m_fB = fB;
	m_fC = fC;

	// mark as unsolved
	m_bUnsolved = true;
}

/***************************************************************************************************
*
*	FUNCTION		CQuadraticSolver::Solve
*
*	DESCRIPTION		Solves the quadratic analytically.
*
***************************************************************************************************/

void CQuadraticSolver::Solve() const
{
	// fall back to a linear equation if |a| =~= 0
	if( fabsf( m_fA ) < EPSILON )
	{
		// solving ( b*x + c = 0 )
		
		// if |b| =~= 0 we can't solve
		if( fabsf( m_fB ) < EPSILON )
		{
			// no solution (malformed equation of the form c = 0 for c =/= 0)
			m_iNumSolutions = 0;
		}
		else
		{
			// otherwise x = -c/b
			m_iNumSolutions = 1;
			m_afSolutions[0] = -m_fC/m_fB;
		}
	}
	else
	{
		// solving ( a*x^2 + b*x + c = 0 )
		
		// check q = b^2 - 4*a*c
		float fSquare = m_fB*m_fB - 4.0f*m_fA*m_fC;
		if(fSquare < 0.0f)
		{
			// q < 0 => no solution
			m_iNumSolutions = 0;
		}
		else if(fSquare == 0.0f)
		{
			// q == 0 => exactly one solution
            m_iNumSolutions = 1;

			// solution is -b/2a
			m_afSolutions[0] = -m_fB/( 2.0f*m_fA );
		}
		else
		{
			// q > 0 => two solutions
			m_iNumSolutions = 2;

			// solutions are x = ( -b + sqrt( q ) )/2*a and x = ( -b - sqrt( q ) )/2*a
			float fDenom = 0.5f/m_fA;
			float fRoot = fsqrtf(fSquare);

			// order the solutions in ascending order
			if( fDenom < 0.0f ) 
			{
				m_afSolutions[0] = ( -m_fB + fRoot )*fDenom;
				m_afSolutions[1] = ( -m_fB - fRoot )*fDenom;
			}
			else
			{
				m_afSolutions[0] = ( -m_fB - fRoot )*fDenom;
				m_afSolutions[1] = ( -m_fB + fRoot )*fDenom;
			}
		}
	}

	// mark as solved
	ntAssert( m_bUnsolved );
	m_bUnsolved = false;
}

/***************************************************************************************************
*
*	FUNCTION		CQuadraticSolver::GetNumSolutions
*
*	DESCRIPTION		Gets the number of solutions of this quadratic. Can be zero.
*
***************************************************************************************************/

int CQuadraticSolver::GetNumSolutions() const
{
	// check we've solved this
	if( m_bUnsolved )
		Solve();

	// return the number of solutions
	return m_iNumSolutions;
}

/***************************************************************************************************
*
*	FUNCTION		CQuadraticSolver::GetSolution
*
*	DESCRIPTION		Gets the solution of given index. Must be >= 0 and < numSolutions.
*
***************************************************************************************************/

float CQuadraticSolver::GetSolution( int iIndex ) const
{
	// check we've solved this
	if( m_bUnsolved )
		Solve();

	// check the index is valid
	ntAssert( 0 <= iIndex && iIndex < m_iNumSolutions );
	
	// return the solution at this index
	return m_afSolutions[iIndex];
}
