#ifndef DD_Image_Deprecation_H
#define DD_Image_Deprecation_H

#include "Build/fnBuild.h"

#if kDDImageVersionMajorNum <= 11
#define mFnDeprecatedInNuke11Func mFnDeprecatedFunc
#endif // nuke 11

#if kDDImageVersionMajorNum <= 12
#define mFnDeprecatedInNuke12Func mFnDeprecatedFunc
#endif // nuke 12

#endif // DD_Image_Deprecation_H
