// MemoryRegistrant.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_MemoryRegistrant_H
#define DDImage_MemoryRegistrant_H

#include "Memory.h"

namespace DD
{
  namespace Image
  {

    template<class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5, class ArgT6, class ArgT7, class ArgT8, class ArgT9, class ArgT10, class ArgT11, class ArgT12> 
    class MemoryHolderFactory;

    // This class is created automatically and should not be instantiated directly
    // Define a template pointer class taking 3 parameters (for testing)
    template<class MemoryHolderT, class ArgT1 = void, class ArgT2 = void, class ArgT3 = void, class ArgT4 = void, class ArgT5 = void, class ArgT6 = void, class ArgT7 = void, class ArgT8 = void, class ArgT9 = void, class ArgT10 = void, class ArgT11 = void, class ArgT12 = void> 
    class MemoryRegistrant : public MemoryHolderT // Note that this is generated locally and should not be exported
    {
    private:
      // Private constructor
      MemoryRegistrant(ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6, ArgT7 t7, ArgT8 t8, ArgT9 t9, ArgT10 t10, ArgT11 t11, ArgT12 t12) : 
        MemoryHolderT(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12) 
      {
        Memory::register_user(this);
      }
      // Private destructor
      virtual ~MemoryRegistrant() 
      { 
        Memory::unregister_user(this);
      }
      
      // Allow the respective smart pointer to create registrants
      friend class MemoryHolderFactory<MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, ArgT6, ArgT7, ArgT8, ArgT9, ArgT10, ArgT11, ArgT12>;
    };

  } // namespace Image
} // namespace DD

#include "MemoryRegistrant_Partial.h"

#endif // DDImage_MemoryRegistrant_H

// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
