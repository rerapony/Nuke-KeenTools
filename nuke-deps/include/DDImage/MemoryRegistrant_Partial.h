// MemoryRegistrant_Partial.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#if !(defined DDImage_MemoryRegistrant_H) || (defined DDImage_MemoryRegistrant_Partial_H)
  #error Only to be included by MemoryRegistrant.h
#endif

#ifndef DDImage_MemoryRegistrant_Partial_H
#define DDImage_MemoryRegistrant_Partial_H

namespace DD
{
  namespace Image
  {
  
#define FN_MEMORYREGISTRANT_SPECIALIZATION                                     \
    template<FN_TEMPLATE_PARAMETER_DECLARATIONS>                              \
    class MemoryRegistrant<FN_TEMPLATE_PARAMETERS> : public MemoryHolderT      \
    {                                                                         \
    private:                                                                  \
      /* Private constructor */                                               \
      MemoryRegistrant(FN_FUNCTION_ARGUMENT_DECLARATIONS) :                    \
        MemoryHolderT(FN_FUNCTION_ARGUMENTS)                                  \
      {                                                                       \
        Memory::register_user(this);                                          \
      }                                                                       \
      /* Private destructor */                                                \
      virtual ~MemoryRegistrant()                                              \
      {                                                                       \
        Memory::unregister_user(this);                                        \
      }                                                                       \
                                                                              \
      /* Allow the respective smart pointer to create registrants */           \
      friend class MemoryHolderFactory<FN_TEMPLATE_PARAMETERS>;               \
    };

// Eleven arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5, class ArgT6, class ArgT7, class ArgT8, class ArgT9, class ArgT10, class ArgT11
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, ArgT6, ArgT7, ArgT8, ArgT9, ArgT10, ArgT11, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6, ArgT7 t7, ArgT8 t8, ArgT9 t9, ArgT10 t10, ArgT11 t11
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11
  
    // Specialise for eleven arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Ten arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5, class ArgT6, class ArgT7, class ArgT8, class ArgT9, class ArgT10
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, ArgT6, ArgT7, ArgT8, ArgT9, ArgT10, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6, ArgT7 t7, ArgT8 t8, ArgT9 t9, ArgT10 t10
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4, t5, t6, t7, t8, t9, t10
  
    // Specialise for ten arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Nine arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5, class ArgT6, class ArgT7, class ArgT8, class ArgT9
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, ArgT6, ArgT7, ArgT8, ArgT9, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6, ArgT7 t7, ArgT8 t8, ArgT9 t9
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4, t5, t6, t7, t8, t9
  
    // Specialise for nine arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Eight arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5, class ArgT6, class ArgT7, class ArgT8
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, ArgT6, ArgT7, ArgT8, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6, ArgT7 t7, ArgT8 t8
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4, t5, t6, t7, t8
  
    // Specialise for eight arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Seven arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5, class ArgT6, class ArgT7
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, ArgT6, ArgT7, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6, ArgT7 t7
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4, t5, t6, t7
  
    // Specialise for seven arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Six arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5, class ArgT6
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, ArgT6, void, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5, ArgT6 t6
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4, t5, t6
  
    // Specialise for six arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Five arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4, class ArgT5
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, ArgT5, void, void, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4, ArgT5 t5
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4, t5
  
    // Specialise for five arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Four arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3, class ArgT4
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, ArgT4, void, void, void, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3, ArgT4 t4
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3, t4
  
    // Specialise for four arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Three arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2, class ArgT3
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, ArgT3, void, void, void, void, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2, ArgT3 t3
#define FN_FUNCTION_ARGUMENTS \
  t1, t2, t3
  
    // Specialise for one argument
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// Two arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1, class ArgT2
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, ArgT2, void, void, void, void, void, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1, ArgT2 t2
#define FN_FUNCTION_ARGUMENTS \
  t1, t2
  
    // Specialise for two arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// One arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT, class ArgT1
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, ArgT1, void, void, void, void, void, void, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS \
  ArgT1 t1
#define FN_FUNCTION_ARGUMENTS \
  t1
  
    // Specialise for one argument
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

// No arguments
#define FN_TEMPLATE_PARAMETER_DECLARATIONS \
  class MemoryHolderT
#define FN_TEMPLATE_PARAMETERS \
  MemoryHolderT, void, void, void, void, void, void, void, void, void, void, void, void
#define FN_FUNCTION_ARGUMENT_DECLARATIONS
#define FN_FUNCTION_ARGUMENTS
  
    // Specialise for no arguments
    FN_MEMORYREGISTRANT_SPECIALIZATION
    
#undef FN_TEMPLATE_PARAMETER_DECLARATIONS
#undef FN_TEMPLATE_PARAMETERS
#undef FN_FUNCTION_ARGUMENT_DECLARATIONS
#undef FN_FUNCTION_ARGUMENTS

  } // namespace Image
} // namespace DD

#endif // DDImage_MemoryRegistrant_Partial_H

// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
