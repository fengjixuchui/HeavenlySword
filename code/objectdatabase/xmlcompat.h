
#undef IHELP
#undef IPREFIX
#undef IFLOAT
#undef IINT
#undef ISTRING
#undef IVECTOR
#undef IPOINT
#undef IQUAT
#undef ILIGHT
#undef IYPR
#undef ICOLOUR
#undef IREFERENCE
#undef IBOOL
#undef IENUM
#undef IFLIPFLOP

#undef _IHELP
#undef _IPREFIX
#undef _IFLOAT
#undef _IINT
#undef _ISTRING
#undef _IVECTOR
#undef _IPOINT
#undef _IQUAT
#undef _ILIGHT
#undef _IYPR
#undef _ICOLOUR
#undef _IREFERENCE
#undef _IBOOL
#undef _IENUM
#undef _IFLIPFLOP

#define IHELP(s)
#define IPREFIX(s)
#define IFLOAT(c,v) PUBLISH_VAR_AS( m_f##v, v )
#define IINT(c,v) PUBLISH_VAR_AS( m_i##v, v )
#define ISTRING(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define IVECTOR(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define IPOINT(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define IQUAT(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define ILIGHT(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define IYPR(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define ICOLOUR(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define IBOOL(c,v) PUBLISH_VAR_AS( m_b##v, v )
#define IENUM(c,v, e) PUBLISH_GLOBAL_ENUM_AS( m_e##v, v, e )
#define IFLIPFLOP(c,v) PUBLISH_VAR_AS( m_ob##v, v )
#define IREFERENCE(c,v) PUBLISH_PTR_AS( m_pob##v, v )

// Defines with defaults
#define IENUM_d(c,v, e, d) PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS( m_e##v, v, e, d )


#define _IFLOAT(v) PUBLISH_VAR_AS( m_f##v, v )
#define _IINT(v) PUBLISH_VAR_AS( m_i##v, v )
#define _ISTRING(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _IVECTOR(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _IPOINT(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _IQUAT(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _ILIGHT(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _IYPR(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _ICOLOUR(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _IBOOL(v) PUBLISH_VAR_AS( m_b##v, v )
#define _IENUM(v, e) PUBLISH_GLOBAL_ENUM_AS( m_e##v, v, e )
#define _IFLIPFLOP(v) PUBLISH_VAR_AS( m_ob##v, v )
#define _IREFERENCE(v) PUBLISH_PTR_AS( m_pob##v, v )

