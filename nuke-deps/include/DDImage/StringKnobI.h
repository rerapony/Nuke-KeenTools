// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_StringKnobI_H
#define DD_Image_StringKnobI_H

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    class OutputContext;
    class Knob;

    class StringKnobI
    {
    public:
      static const int kNoSelection = -1;

      typedef void (CursorPositionChangedCallback)(Knob* knob, int position, int selectionStart, int selectionEnd);


      // TODO:: make functions pure virtual so as to be in agreement with
      // rest of DDImage::*I interface classes 
      // - no change in functionality should be observed in making them pure virtual -
      virtual void setCursorPosition(int newPosition, bool moveAnchor)
      {
      }

      virtual void registerCursorChangedCallback(CursorPositionChangedCallback* cb)
      {
      }

      virtual void default_value(const char* s, bool store = false)
      {
      }

      virtual ~StringKnobI() {}
    };
  }
}

#endif  // DD_Image_StringKnobI_H
