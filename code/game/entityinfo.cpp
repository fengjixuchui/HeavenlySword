/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

// Necessary includes
#include "game/entityinfo.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/luaattrtable.h"
#include "game/aicomponent.h"
#include "game/attacks.h" // To check if our state is CS_DYING and therefore we're still lockonable

// Stuff for debug render only, remove if that shifts
#include "core/timer.h"
#include "core/visualdebugger.h"
#include "input/inputhardware.h"
#include "objectdatabase/dataobject.h"

#include "luaexptypes.h"
#include "lua/ninjalua.h"

/***************************************************************************************************
* Start exposing the elements to Lua
***************************************************************************************************/

