//
// Created by wangrl on 2020/12/9.
//

#pragma once

#if !defined(WTF_IMPLEMENTATION)
#define WTF_IMPLEMENTATION 0
#endif

#if defined(COMPONENT_BUILD)
#if defined(WIN32)
#if WTF_IMPLEMENTATION
#define WTF_EXPORT __declspec(dllexport)
#else
#define WTF_EXPORT __declspec(dllimport)
#endif
#else // defined(WIN32)
#define WTF_EXPORT __attribute__((visibility("default")))
#endif
#else // defined(COMPONENT_BUILD)
#define WTF_EXPORT
#endif
