//--------------------------------------------------
//!
//!	\file cloudArray.inl
//!	cloudArray

//! TODO Deano
//! This file need renameing and clarifing if its to stay in core!
//!
//--------------------------------------------------

#ifndef CORE_BOOSTARRAY_INL_
#define CORE_BOOSTARRAY_INL_


#include "core/boostarray.h"
#include "numerics.h"






///////////////////////////////////////////////
// Array<T,N> definition



namespace ArrayOp
{
	/// \f$ v *= a \f$ 
	template<typename T, int N>
	void v_teq ( Array<T,N>& y, const T& a )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		for( ; Yi!=Yend; ++Yi )
			(*Yi) *= a;
	}

	/// \f$ v *= a \f$ 
	template<typename T, int N>
	void v_teq ( Array<T,N>& y, const Array<T,N>& x )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		typename Array<T,N>::const_iterator Xi = x.begin();
		for( ; Yi!=Yend; ++Yi, ++Xi )
			(*Yi) *= (*Xi);
	}

	/// compute the sum of vector elements
	template<typename T, int N>
	T v_sum( const Array<T,N>& x )
	{
		typename Array<T,N>::const_iterator xi = x.begin(), xend=x.end();
		T d = 0;
		for( ; xi!=xend; ++xi )
			d += (*xi);
		return d;
	}

	/// compute the square of the sum of vector elements
	template<typename T, int N>
	T v_sqsum( const Array<T,N>& x )
	{
		typename Array<T,N>::const_iterator xi = x.begin(), xend=x.end();
		T d = 0;
		for( ; xi!=xend; ++xi )
			d += (*xi)*(*xi);
		return d;
	}

	/// compute the mean of vector elements
	template<typename T, int N>
	T v_mean( const Array<T,N>& x )
	{
		return v_sum(x)/size(x);
	}

	/// compute the variance of vector elements
	template<typename T, int N>
	T v_variance( const Array<T,N>& x )
	{
		typename Array<T,N>::const_iterator xi = x.begin(), xend=x.end();
		T m = v_mean(x);
		T d = 0;
		for( ; xi!=xend; ++xi )
		{
			T tmp = (*xi)-m;
			tmp*=tmp;
			d+=tmp;
		}
		return d/size(x);
	}


	/// \f$ y = x \f$ 
	template<typename T, int N>
	void v_eq ( Array<T,N>& y, const Array<T,N>& x )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		typename Array<T,N>::const_iterator Xi = x.begin();
		for( ; Yi!=Yend; ++Yi, ++Xi )
			(*Yi) = (*Xi);
	}

	///   \f$  y = -x \f$ 
	template<typename T, int N>
	void v_eq_minus ( Array<T,N>& y, const Array<T,N>& x )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		typename Array<T,N>::const_iterator Xi = x.begin();
		for( ; Yi!=Yend; ++Yi, ++Xi )
			(*Yi) = -(*Xi);
	}


	///  \f$ y += x \f$ 
	template<typename T, int N>
	void v_peq ( Array<T,N>& y, const Array<T,N>& x )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		typename Array<T,N>::const_iterator Xi = x.begin();
		for( ; Yi!=Yend; ++Yi, ++Xi )
			(*Yi) += (*Xi);
	}
	
	///  \f$ y += x \f$ 
	template<typename T, int N>
	void v_peq ( Array<T,N>& y, const T& x )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		for( ; Yi!=Yend; ++Yi )
			(*Yi) += x;
	}
	
	///  \f$ y += x \f$ 
	template<typename T, int N>
	void v_modeq ( Array<T,N>& y, const T& x )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		for( ; Yi!=Yend; ++Yi )
			(*Yi) %= x;
	}

	///  \f$ y -= x \f$ 
	template<typename T, int N>
	void v_meq ( Array<T,N>& y, const Array<T,N>& x )
	{
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		typename Array<T,N>::const_iterator Xi = x.begin();
		for( ; Yi!=Yend; ++Yi, ++Xi )
			(*Yi) -= (*Xi);
	}

	///  \f$ y += aX \f$ 
	template<typename T, int N>
	void v_peq ( Array<T,N>& y, T a, const Array<T,N>& x )
	{
		typename Array<T,N>::iterator yi = y.begin(), yend=y.end();
		typename Array<T,N>::const_iterator xi = x.begin();
		for( ; yi!=yend; ++xi, ++yi )
			(*yi) += a * (*xi);
	}

	/// Cross product of two vectors:  \f$  u = v \times w  \f$  (generalisation of cross product in dimension 3). (Precond: &u!=&v and &u!=&w)
	template<typename T, int N>
	Array<T,N> v_cross ( const Array<T,N>& a, const Array<T,N>& b )
	{
		Array<T,N> res;
		v_eq_cross(res,a,b);
		return res;
	}

	/// Cross product of two vectors:  \f$  u = v \times w  \f$  (generalisation of cross product in dimension 3). (Precond: &u!=&v and &u!=&w)
	template<typename T, int N>
	void v_eq_cross ( Array<T,N>&u, const Array<T,N>& v, const Array<T,N>& w )
	{
		ntAssert( static_cast<const void*>(&u) != static_cast<const void*>(&v) );
		ntAssert( static_cast<const void*>(&u) != static_cast<const void*>(&w) );
		const int n = size(u);
		for( int i=0, iend=n; i!=iend; ++i )
			u[(i+2)%n] = v[i]*w[(i+1)%n] -v[(i+1)%n]*w[i];
	}

	/// dot product of two vectors
	template<typename T, int N>
	T v_dot( const Array<T,N>& x, const Array<T,N>& y )
	{
		typename Array<T,N>::const_iterator yi = y.begin(), yend=y.end();
		typename Array<T,N>::const_iterator xi = x.begin();
		T d = 0;
		for( ; yi!=yend; ++xi, ++yi )
			d += (*yi) * (*xi);
		return d;
	}


	/// compute norm 2 of vector
	template<typename T, int N>
	T v_norm( const Array<T,N>& v )
	{
		return Numerics<T>::sqrt(v_sqsum(v));
	}

	/// compute norm 2 of vector
	template<typename T, int N>
	T v_sqnorm( const Array<T,N>& v )
	{
		return v_sqsum(v);
	}

	/// compute norm 1 of vector
	template<typename T, int N>
	T v_norm1( const Array<T,N>& x )
	{
		typename Array<T,N>::const_iterator xi = x.begin(), xend=x.end();
		T d = 0;
		for( ; xi!=xend; ++xi )
			d += Numerics<T>::abs(*xi);
		return d;
	}

	/// compute norm infinite of vector
	template<typename T, int N>
	T v_normInf( const Array<T,N>& x )
	{
		typename Array<T,N>::const_iterator xi = x.begin(), xend=x.end();
		T d = 0;
		for( ; xi!=xend; ++xi )
			if( Numerics<T>::abs(*xi) > d ) d = Numerics<T>::abs(*xi);
		return d;
	}

	/// compute norm infinite of vector
	template<typename T, int N>
	T v_min( const Array<T,N>& x )
	{
		typename Array<T,N>::const_iterator xi = x.begin(), xend=x.end();
		T d = *xi; ++xi;
		for( ; xi!=xend; ++xi )
			if( (*xi) < d ) d = (*xi);
		return d;
	}
	
	/// compute norm infinite of vector
	template<typename T, int N>
	T v_max( const Array<T,N>& x )
	{
		typename Array<T,N>::const_iterator xi = x.begin(), xend=x.end();
		T d = *xi; ++xi;
		for( ; xi!=xend; ++xi )
			if( (*xi) > d ) d = (*xi);
		return d;
	}




///////////////////////////























	// comparisons
	template<typename T, int N>
	bool operator== (const Array<T,N>& x,const Array<T,N>& y)
	{
		for(int i = 0 ; i < N ; i++ )
		{
			if (x[i]!=y[i]) return false;
		}
		return true;
	}

	template<typename T, int N>
	bool operator< (const Array<T,N>& x,const Array<T,N>& y)
	{
		bool res = true;
		for(int i = 0 ; i < N ; i++ )
		{
			res=res && (x[i]<y[i]);
		}
		// equal
		return res;
	}

	template<typename T, int N>
	bool operator> (const Array<T,N>& x,const Array<T,N>& y)
	{
		bool res = true;
		for(int i = 0 ; i < N ; i++ )
		{
			res=res && (x[i]>y[i]);
		}
		// equal
		return res;
	}

	template<typename T, int N>
	bool operator<= (const Array<T,N>& x,const Array<T,N>& y)
	{
		bool res = true;
		for(int i = 0 ; i < N ; i++ )
		{
			res=res && (x[i]<=y[i]);
		}
		// equal
		return res;
	}

	template<typename T, int N>
	bool operator>= (const Array<T,N>& x,const Array<T,N>& y)
	{
		bool res = true;
		for(int i = 0 ; i < N ; i++ )
		{
			res=res && (x[i]>=y[i]);
		}
		// equal
		return res;
	}



	/// \f$ u += v \f$
	template<typename T, int N>
	Array<T,N>& operator += (Array<T,N>& t, const Array<T,N>& v )
	{
		ArrayOp::v_peq(t,v);
		return t;
	}

	/// \f$ u -= v \f$
	template<typename T, int N>
	Array<T,N>& operator -= (Array<T,N>& t, const Array<T,N>& v )
	{
		ArrayOp::v_meq(t,v);
		return t;
	}

	/// \f$ u *= a \f$ where a is a scalar
	template<typename T, int N>
	Array<T,N>& operator *= (Array<T,N>& t, const T& a )
	{
		ArrayOp::v_teq(t,a);
		return t;
	}
	
	/// \f$ u += a \f$ where a is a scalar
	template<typename T, int N>
	Array<T,N>& operator += (Array<T,N>& t, const T& a )
	{
		ArrayOp::v_peq(t,a);
		return t;
	}
	
	/// \f$ u %= a \f$ where a is a scalar
	template<typename T, int N>
	Array<T,N>& operator %= (Array<T,N>& t, const T& a )
	{
		ArrayOp::v_modeq(t,a);
		return t;
	}

	/// \f$ u *= a \f$ where a is a scalar
	template<typename T, int N>
	Array<T,N>& operator *= (Array<T,N>& t, const Array<T,N>& v )
	{
		ArrayOp::v_teq(t,v);
		return t;
	}


	/// \f$ u * a \f$ where a is a scalar
	template<typename T, int N>
	Array<T,N> operator * ( const Array<T,N>& t, const T& a )
	{
		Array<T,N> res = t;
		res*=a;
		return res;
	}

	/// \f$ u * v \f$ where v is an Array
	template<typename T, int N>
	Array<T,N> operator * ( const Array<T,N>& t, const Array<T,N>& v )
	{
		Array<T,N> res = t;
		res*=v;
		return res;
	}

	/// \f$ -u \f$
	template<typename T, int N>
	Array<T,N> operator - (const Array<T,N>& t)
	{
		Array<T,N> y(t);
		typename Array<T,N>::iterator Yi = y.begin(), Yend=y.end();
		for( ; Yi!=Yend; ++Yi )
			(*Yi) = -(*Yi);
		return y;
	}
		



	/// \f$ u + v \f$
	template<typename T, int N>
	Array<T,N> operator + (const Array<T,N>& t, const Array<T,N>& v)
	{
		Array<T,N> w = (t);
		ArrayOp::operator += (w,v);
		return w;
	}

	/// \f$ u - v \f$
	template<typename T, int N>
	Array<T,N> operator - (const Array<T,N>& t, const Array<T,N>& v )
	{
		Array<T,N> w(t);
		ArrayOp::operator -= (w,v);
		return w;
	}
	/// \f$ a * u \f$ where a is a scalar
	template<typename T, int N>
	inline Array<T,N> operator * ( const T& a, const Array<T,N>& v )
	{
		Array<T,N> w = v;
		return w*=a;
	}













/////////////////////////////////////////////////////////




} // ArrayOp

using namespace ArrayOp;


#endif // end of _CLOUDARRAY_INL_
