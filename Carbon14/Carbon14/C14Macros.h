
#ifndef __C14Macros_h__
#define __C14Macros_h__


#if BUILDING_CARBON_14
#define C14_API(type) DEFINE_API(type)
#else
#define C14_API(type) EXTERN_API(type)
#endif


#endif /* __C14Macros_h__ */
