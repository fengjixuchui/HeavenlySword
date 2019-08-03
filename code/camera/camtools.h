/***************************************************************************************************
*
* Lets kill this file...
*
***************************************************************************************************/

#ifndef CAM_TOOLS_H
#define CAM_TOOLS_H

#ifdef CAMERA_PRINTF
#define LOG_WARNING(msg) ntPrintf("%s(%d) : %s\n", __FILE__, __LINE__, msg)
#else
#define LOG_WARNING(msg)
#endif

#endif // CAM_TOOLS_H
