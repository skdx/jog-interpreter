#include <sys/time.h>

#include "jog.h"

// See add_native_handlers() at bottom.

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

void System__currentTimeMillis( JogVM* vm )
{
  vm->pop_frame();

  struct timeval time_struct;
  JogInt64 time_ms;
  gettimeofday( &time_struct, 0 );
  time_ms = (JogInt64) time_struct.tv_sec;
  time_ms *= 1000;
  time_ms += (time_struct.tv_usec / 1000);

  vm->push( time_ms );
}


void JogVM::add_native_handlers()
{
  add_native_handler( "PrintWriter::print(boolean)", PrintWriter__print__boolean );
  add_native_handler( "PrintWriter::print(char)", PrintWriter__print__char );
  add_native_handler( "PrintWriter::print(double)", PrintWriter__print__double );
  add_native_handler( "PrintWriter::print(int)", PrintWriter__print__int );
  add_native_handler( "PrintWriter::print(long)", PrintWriter__print__long );
  add_native_handler( "PrintWriter::print(String)", PrintWriter__print__String );
  add_native_handler( "System::currentTimeMillis()", System__currentTimeMillis );
}

