// MemoryHolderFactory.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_MemoryHolderFactory_H
#define DDImage_MemoryHolderFactory_H

#include "MemoryRegistrant.h"
#include "DDImage/Deprecation.h"

namespace DD
{
  namespace Image
  {

    // Define a template pointer class taking 3 parameters
    template<class MemoryHolderT, class ArgT1 = void, class ArgT2 = void, class ArgT3 = void, class ArgT4 = void, class ArgT5 = void, class ArgT6 = void, class ArgT7 = void, class ArgT8 = void, class ArgT9 = void, class ArgT10 = void, class ArgT11 = void, class ArgT12 = void>
    class MemoryHolderFactory // Note that this is generated locally and should not be exported
    {
    public:
      MemoryHolderFactory() {}
      ~MemoryHolderFactory() {}

      // When this function is removed, due to deprecation, the whole MemoryHolderFactor and
      // MemoryRegistrant classes should be removed.
      mFnDeprecatedInNuke12Func("MemoryHolderFactory<Type, Args...> has been replaced with MemHolderFactory<Type>. Use as a drop-in replacement",
        static MemoryHolderT* create(ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6, ArgT7 t7, ArgT8 t8, ArgT9 t9, ArgT10 t10, ArgT11 t11, ArgT12 t12);
      )

    protected:
      // Not implemented
      MemoryHolderFactory(const MemoryHolderFactory& rhs);
      MemoryHolderFactory& operator=(const MemoryHolderFactory& rhs);
    };

    template<class MemoryHolderT, class ArgT1, class ArgT2,  class ArgT3,  class ArgT4,
                                  class ArgT5, class ArgT6,  class ArgT7,  class ArgT8,
                                  class ArgT9, class ArgT10, class ArgT11, class ArgT12>
    /*static*/ MemoryHolderT* MemoryHolderFactory<
      MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4,
                     ArgT5, ArgT6, ArgT7, ArgT8,
                     ArgT9, ArgT10, ArgT11, ArgT12
    >::create(
      ArgT1 t1, ArgT2  t2,  ArgT3  t3,  ArgT4  t4,
      ArgT5 t5, ArgT6  t6,  ArgT7  t7,  ArgT8  t8,
      ArgT9 t9, ArgT10 t10, ArgT11 t11, ArgT12 t12
    )
    {
      return new MemoryRegistrant<MemoryHolderT, ArgT1, ArgT2,  ArgT3,  ArgT4,
                                                 ArgT5, ArgT6,  ArgT7,  ArgT8,
                                                 ArgT9, ArgT10, ArgT11, ArgT12> (
        t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12
      );
    }

  } // namespace Image
} // namespace DD

#include "MemoryHolderFactory_Partial.h"

#endif // DDImage_MemoryHolderFactory_H

// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
