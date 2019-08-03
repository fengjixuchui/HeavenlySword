#include "core/explicittemplate.h"

#include "core/boostarray.inl"
//#include "polynom.inl"
#include "core/timeinfo.inl"


////////////////////////////////////////////////////
// Group those declare in explicittemplate.h
template class Array<int,2>;
template class Array<int,3>;
template class Array<int,4>;
template class Array<float,2>;
template class Array<float,3>;
template class Array<float,4>;
template class Array<float,16>;
// End of Group those declare in explicittemplate.h
///////////////////////////////////////////////////


///////////////////////////////////////////////////
// Group others
template class TemplateTimeInfo<3>;
//template class TPolynom<float>;
template class Array<CVector,3>;
// End of Group others
///////////////////////////////////////////////////

