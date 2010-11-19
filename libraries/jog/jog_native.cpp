#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <cmath>
using namespace std;

#if !defined(_WIN32)
#  include <sys/time.h>
#endif

#include "jog.h"

// See add_native_handlers() at bottom.

//=============================================================================
//  Math
//=============================================================================
void Math__floor__double( JogVM* vm )
{
  double n = vm->pop_double();
  vm->pop_frame();
  vm->push( floor(n) );
}

//=============================================================================
//  PrintWriter
//=============================================================================
void PrintWriter__print__boolean( JogVM* vm )
{
  if (vm->pop_int()) printf("true");
  else               printf("false");
}

void PrintWriter__print__char( JogVM* vm )
{
  putchar( vm->pop_int() );
}

void PrintWriter__print__double( JogVM* vm )
{
  printf("%lf",vm->pop_double());
}

void PrintWriter__print__int( JogVM* vm )
{
  printf("%d",vm->pop_int());
}

void PrintWriter__print__long( JogVM* vm )
{
  printf("%lld",vm->pop_long());
}

void PrintWriter__print__String( JogVM* vm )
{
  JogObject* str = *(vm->pop_ref());
  if (str == NULL)
  {
    printf( "null" );
  }
  else
  {
    JogObject* array = *((JogObject**)&(str->data[0]));
    if (array)
    {
      int count = array->count + 1;
      JogChar* cur  = ((JogChar*) array->data) - 1;
      while (--count)
      {
        putchar( *(++cur) );
      }
    }
    else
    {
      printf( "[Internal] null array on String print." );
    }
  }
}

//=============================================================================
//  System
//=============================================================================
void System__currentTimeMillis( JogVM* vm )
{
  vm->pop_frame();

#if defined(_WIN32)
  struct __timeb64 time_struct;
  JogInt64 time_ms;
  _ftime64_s( &time_struct );
  time_ms = (JogInt64) time_struct.time;
  time_ms *= 1000;
  time_ms += time_struct.millitm;
#else
  struct timeval time_struct;
  JogInt64 time_ms;
  gettimeofday( &time_struct, 0 );
  time_ms = (JogInt64) time_struct.tv_sec;
  time_ms *= 1000;
  time_ms += (time_struct.tv_usec / 1000);
#endif

  vm->push( time_ms );
}


void JogVM::add_native_handlers()
{
  add_native_handler( "Math::floor(double)", Math__floor__double );
  add_native_handler( "PrintWriter::print(boolean)", PrintWriter__print__boolean );
  add_native_handler( "PrintWriter::print(char)", PrintWriter__print__char );
  add_native_handler( "PrintWriter::print(double)", PrintWriter__print__double );
  add_native_handler( "PrintWriter::print(int)", PrintWriter__print__int );
  add_native_handler( "PrintWriter::print(long)", PrintWriter__print__long );
  add_native_handler( "PrintWriter::print(String)", PrintWriter__print__String );
  add_native_handler( "System::currentTimeMillis()", System__currentTimeMillis );
}

