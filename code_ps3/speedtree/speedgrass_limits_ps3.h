#ifndef SPEEDGRASS_LIMITS_PS3_H
#define SPEEDGRASS_LIMITS_PS3_H

template <class T>
class CLimits
{
public:
	CLimits() {}

	CLimits(const T& min, const T& max)
		: min_(min)
		, max_(max)
	{
	}

	T	min_;
	T	max_;

};

template <>
class CLimits<float [2]>
{
public:
	CLimits() {}

	CLimits(const float min[2], const float max[2])
	{
		min_[0] = min[0];
		min_[1] = min[1];

		max_[0] = max[0];
		max_[1] = max[1];
	}
	float min_[2];
	float max_[2];
};

template<class T>
inline CLimits<T> Limits(const T& min, const T& max)
{
	return CLimits<T>(min, max);
}


#endif
