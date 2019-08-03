#include "ainavtriangulate.h"
#include "game/randmanager.h"
#include "core/maths.h"

static int choose_idx;
static int permute[SEGSIZE];


/* Generate a random permutation of the segments 1..n */
int generate_random_ordering(
     int n)
{
  //struct timeval tval;
  //struct timezone tzone;
  register int i;
  int m, st[SEGSIZE], *p;
  
  choose_idx = 1;
  //gettimeofday(&tval, &tzone);
  //srand48(tval.tv_sec);

  for (i = 0; i <= n; i++)
    st[i] = i;

  p = st;
  for (i = 1; i <= n; i++, p++)
    {
      //m = lrand48() % (n + 1 - i) + 1;
      m = grand() % (n + 1 - i) + 1;
      permute[i] = p[m];
      if (m != 1)
	p[m] = p[1];
    }
  return 0;
}

  
/* Return the next segment in the generated random ordering of all the */
/* segments in S */
int choose_segment()
{
#ifdef _TRIANGULATE_DEBUG
  ntPrintf( "choose_segment: %d\n", permute[choose_idx]);
#endif 
  return permute[choose_idx++];
}


#ifdef STANDALONE

/* Read in the list of vertices from infile */
int read_segments(filename, genus)
     char *filename;
     int *genus;
{
  FILE *infile;
  int ccount;
  register int i;
  int ncontours, npoints, first, last;

  if ((infile = fopen(filename, "r")) == NULL)
    {
      perror(filename);
      return -1;
    }

  fscanf(infile, "%d", &ncontours);
  if (ncontours <= 0)
    return -1;
  
  /* For every contour, read in all the points for the contour. The */
  /* outer-most contour is read in first (points specified in */
  /* anti-clockwise order). Next, the inner contours are input in */
  /* clockwise order */

  ccount = 0;
  i = 1;
  
  while (ccount < ncontours)
    {
      int j;

      fscanf(infile, "%d", &npoints);
      first = i;
      last = first + npoints - 1;
      for (j = 0; j < npoints; j++, i++)
	{
	  fscanf(infile, "%lf%lf", &seg[i].v0.x, &seg[i].v0.y);
	  if (i == last)
	    {
	      seg[i].next = first;
	      seg[i].prev = i-1;
	      seg[i-1].v1 = seg[i].v0;
	    }
	  else if (i == first)
	    {
	      seg[i].next = i+1;
	      seg[i].prev = last;
	      seg[last].v1 = seg[i].v0;
	    }
	  else
	    {
	      seg[i].prev = i-1;
	      seg[i].next = i+1;
	      seg[i-1].v1 = seg[i].v0;
	    }
	  
	  seg[i].is_inserted = FALSE;
	}

      ccount++;
    }

  *genus = ncontours - 1;
  return i-1;
}

#endif



/***************************************************************************************************
*
*	FUNCTION		log2f
*
*	DESCRIPTION		Returns the binary logarithm of the input value
*
*	NOTES			Range is 1.17549e-038 to +FLT_MAX4
*
***************************************************************************************************/
/*
_PS_CONST( log2_c0,	float,	1.44269504088896340735992f );

static float	log2f( float fInput )
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

*/

const float	MATH_ZERO = 0.0f;
const float	MATH_ONE = 1.0f;

#define	MATH_S16_TO_F32( s16_in, f32_out ) \
	((f32_out) = (float)(s16_in))

#define IEEE_EXPONENT_BITS              0x7f800000
#define IEEE_MANTISSA_LENGTH            23
#define IEEE_EXPONENT_BIAS              127
#define IEEE_MASK_BITS                  0x007FFFFF
#define IEEE_ONE_BITS                   0x3F800000

// Extract the bits of an ieee float
#define ieeeFloatToBits( f )                 (*(unsigned int*) &(f))

// Extract the exponent of an ieee floats bit representation
#define ieeeFloatExponent( u )               (short)((((u) & IEEE_EXPONENT_BITS) >> IEEE_MANTISSA_LENGTH) - IEEE_EXPONENT_BIAS)

// Normalise the bits of an ieee floats bit representation
#define ieeeFloatNormalise( u )              (((u) & IEEE_MASK_BITS) | IEEE_ONE_BITS)


// Coefficents for poly 8
#define LOG8STD0    4.88635801820791470000e-008f
#define LOG8STD1    1.44268677782597490000e+000f
#define LOG8STD2    -7.21114614403517540000e-001f
#define LOG8STD3    4.78323544868638480000e-001f
#define LOG8STD4    -3.45996012436284920000e-001f
#define LOG8STD5    2.39231662977817010000e-001f
#define LOG8STD6    -1.34534254204152240000e-001f
#define LOG8STD7    5.02775073734638630000e-002f
#define LOG8STD8    -8.87469665188802930000e-003f


#define poly8Log( x ) \
    (LOG8STD0 + (x) * (LOG8STD1 + (x) * (LOG8STD2 + (x) * (LOG8STD3 + \
    (x) * (LOG8STD4 + (x) * (LOG8STD5 + (x) * (LOG8STD6 + (x) * (LOG8STD7 + \
    (x) * LOG8STD8))))))))


// This implementation of log2f requires strict IEEE floating point compliance. Works on PC.
// Fortunately, this is a preprocess step, and so this code will eventually be moved from runtime
// to the tools pipeline

#ifdef PLATFORM_PC
float
log2f(
	const float		x
)
{
	float			result;
	float			r;
	float			floatB;
	short			b;
	unsigned int	xBits;
	float			xMantissa;

	ntAssert( x > MATH_ZERO );

	// Break x into exponent and mantissa
	xBits = ieeeFloatToBits( x );
	b = ( short ) ieeeFloatExponent( xBits );
	ieeeFloatToBits( xMantissa ) = ieeeFloatNormalise( xBits );
	r = xMantissa - MATH_ONE;

	MATH_S16_TO_F32( b, floatB );

	// Do polynomial approx. with mantissa of x
	result = poly8Log( r ) + floatB;
	return result;
}
#endif



/* Get log*n for given n */
int math_logstar_n(
     int n)
{
  register int i;
  float v;
  
  for (i = 0, v = (float) n; v >= 1; i++)
    v = log2f(v);
  
  return (i - 1);
}
  

int math_N(
     int n,
     int h)
{
  register int i;
  float v;

  for (i = 0, v = (float) n; i < h; i++)
    v = log2f(v);
  
  return (int) ceil((double) 1.0*n/v);
}
