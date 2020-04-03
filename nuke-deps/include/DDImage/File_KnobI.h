// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef FILE_KNOBI_H
#define FILE_KNOBI_H

namespace DD
{
  namespace Image
  {

    /**
     * Abstract base class for File Knob-specific methods.
     */
    class File_KnobI
    {
    public:
      virtual ~File_KnobI() { }

      /**
       * This enum gives File knobs information about what triggered the last filename change
       */
      enum ChangeContext
      {
        ChangedFromScript = 0,        // Change came from a from_script call
        ChangedFromUser,              // The change ocurred due to direct user intervention (g, typing/browsing a filename)
        ChangedFromUserOverride,      // Force the knob to "think" the change came from user intervention.
                                      // If you set this before a call to fropm_script, internally this will force the last
                                      // change to be ChangedFromUser
        ChangedFromScriptOverride,    // As above but force a ChangedFromScript state.
        ChangedNone
      };

      /**
       * Get the reason for the last change to this knob's value
       */
      virtual ChangeContext getLastChangeContext() const = 0;
      virtual void cooked_frame(const OutputContext& context, int& frame, bool& black, int& offset) = 0;
      virtual bool isReadKnob() const = 0;
    };
  }
}

#endif //FILE_KNOBI_H

