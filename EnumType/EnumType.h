#ifndef __ENUM_TYPE_H__
#define __ENUM_TYPE_H__

#include <string.h>

#define ENUM_VALUE(name,assign) name assign,
#define ENUM_CASE(name,assign) case name: return #name;
#define ENUM_STRCMP(name,assign) if(!strcmp(str,#name)) return name;

/* declare the access function and define enum values */
#define DECLARE_ENUM(EnumType,ENUM_DEF)\
  enum EnumType{ \
    ENUM_DEF(ENUM_VALUE) \
  }; \
  const char *GetString(enum EnumType dummy); \
  enum EnumType Get##EnumType##Value(const char *string); \

/* define the access function names */
#define DEFINE_ENUM(EnumType,ENUM_DEF) \
  const char *GetString(enum EnumType value) \
  { \
    switch(value) \
    { \
      ENUM_DEF(ENUM_CASE) \
      default: return ""; \
    } \
  } \
  enum EnumType Get##EnumType##Value(const char *str) \
  { \
    ENUM_DEF(ENUM_STRCMP) \
    return (enum EnumType)0; \
  } \

#define ENUM_DEFINE(EnumType,ENUM_DEF) \
  DECLARE_ENUM(EnumType,ENUM_DEF) \
  DEFINE_ENUM(EnumType,ENUM_DEF)  \


#endif /* __ENUM_TYPE_H__ */
