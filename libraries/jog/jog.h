
// lexical structure:
//   http://java.sun.com/docs/books/jls/third_edition/html/lexical.html

#ifndef JOG_H
#define JOG_H

// 2010.05.17 by Abe Pralle

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
using namespace std;

#include "ref_counted.h"
#include "string_builder.h"
#include "array_list.h"

typedef long long int JogInt64;
typedef int           JogInt32;
typedef short int     JogInt16;
typedef char          JogInt8;
typedef unsigned short int JogChar;

struct JogMethodInfo;

struct ASCIIString : RefCounted
{
  char* data;
  int   count;

  ASCIIString( const char* st )
  {
    if (st)
    {
      count = strlen(st);
      data = new char[count+1];
      int c = count + 1;
      char* dest = data-1;
      st--;
      while (--c) *(++dest) = *(++st);
      data[count] = 0;
    }
    else
    {
      count = 0;
      data = NULL;
    }
  }

  ~ASCIIString()
  {
    if (data) delete data;
  }

  bool operator==( Ref<ASCIIString> other )
  {
    return 0 == strcmp(data,other->data);
  }

  bool operator!=( Ref<ASCIIString> other )
  {
    return 0 != strcmp(data,other->data);
  }
};

struct JogString : RefCounted
{
  short int* data;
  int   count;

  JogString( const char* st )
  {
    if (st)
    {
      count = strlen(st);
      data = new short int[count];
      int c = count + 1;
      short int* dest = data-1;
      st--;
      while (--c) *(++dest) = *(++st);
    }
    else
    {
      count = 0;
      data = NULL;
    }
  }

  JogString( short int* st, int _count=-1 )
  {
    if (st)
    {
      count = _count;
      if (count == -1)
      {
        count = 0;
        while (st[count]) ++count;
      }

      data = new short int[count];
      int c = count + 1;
      short int* dest = data-1;
      st--;
      while (--c) *(++dest) = *(++st);
    }
    else
    {
      count = 0;
      data = NULL;
    }
  }

  JogString( Ref<JogString> other )
  {
    if (*other)
    {
      count = other->count;
      data = new short int[count];
      memcpy( data, other->data, count*2 );
    }
    else
    {
      count = 0;
      data = NULL;
    }
  }

  ~JogString()
  {
    if (data) delete data;
  }

  /*
  int operator[]( int index )
  {
    return data[index];
  }
  */

  int get( int index )
  {
    if (index < 0) index += count;
    return data[index];
  }

  Ref<ASCIIString> to_ascii()
  {
    StringBuilder buffer;
    for (int i=0; i<count; ++i)
    {
      buffer.print( (char) data[i] );
    }
    return new ASCIIString(buffer.to_string());
  }

  void print()
  {
    for (int i=0; i<count; ++i)
    {
      putc( data[i], stdout );
    }
  }

  void print( StringBuilder &buffer )
  {
    for (int i=0; i<count; ++i)
    {
      buffer.print((char)data[i]);
    }
  }

  void print( UnicodeStringBuilder &buffer )
  {
    for (int i=0; i<count; ++i)
    {
      buffer.print(data[i]);
    }
  }

  long long int to_int64()
  {
    long long int value = 0;
    for (int i=0; i<count; ++i)
    {
      value <<= 4;
      int ch;
      ch = data[i];
      if (ch <= '9') value |= (ch - '0');
      else           value |= (ch - ('A'-10));
    }
    return value;
  }

  int compare_to( Ref<JogString> other_ref )
  {
    JogString* other = *other_ref;

    int c = other->count;
    if (c > count) c = count;

    short int* ptr1 = data - 1;
    short int* ptr2 = other->data - 1;

    ++c;
    while (--c)
    {
      if (*(++ptr1) != *(++ptr2))
      {
        if (*ptr1 < *ptr2) return int(data - ptr1) - 1;
        else               return int(ptr1 - data) + 1;
      }
    }

    if (count < other->count)      return -(count + 1);
    else if (other->count < count) return other->count + 1;

    return 0;
  }

  void add( const char* st )
  {
    int len = strlen(st);

    short int* new_data = new short int[count + len];

    short int* dest = new_data - 1;

    {
      short int* src  = data - 1;
      int c = count + 1;
      while (--c) *(++dest) = *(++src);
    }

    {
      const char* src = st - 1;
      int c = len + 1;
      while (--c) *(++dest) = *(++src);
    }

    if (data) delete data;
    data = new_data;
    count += len;
  }

  void add( Ref<JogString> other )
  {
    if (other->count == 0) return;

    short int* new_data = new short int[count + other->count];

    short int* src  = data - 1;
    short int* dest = new_data - 1;
    int c = count + 1;
    while (--c) *(++dest) = *(++src);

    src  = other->data - 1;
    c = other->count + 1;
    while (--c) *(++dest) = *(++src);

    if (data) delete data;
    data = new_data;
    count += other->count;
  }

  bool equals( Ref<JogString> other )
  {
    if (count != other->count) return false;

    short int* st1 = data - 1;
    short int* st2 = other->data - 1;
    int c = count + 1;
    while (--c)
    {
      if (*(++st1) != *(++st2)) return false;
    }
    return true;
  }

  bool equals( const char* st )
  {
    int len = strlen(st);
    if (count != len) return false;

    short int* st1 = data - 1;
    const char* st2 = st - 1;
    int c = count + 1;
    while (--c)
    {
      if (*(++st1) != *(++st2)) return false;
    }
    return true;
  }

  Ref<JogString> before_first( int ch )
  {
    int i = 0;
    while (i < count)
    {
      if (data[i] == ch) break;
      ++i;
    }
    return new JogString( data, i );
  }

  Ref<JogString> substring( int i1 )
  {
    return new JogString( data+i1, count-i1 );
  }

  Ref<JogString> substring( int i1, int i2 )
  {
    return new JogString( data+i1, (i2-i1)+1 );
  }
};

struct JogStringComparator
{
  bool operator()( Ref<JogString> a, Ref<JogString> b )
  {
    return a->compare_to(b) < 0;
  }
};


struct JogError : RefCounted
{
  Ref<ASCIIString> message;
  Ref<RefCounted> reader;
  int line, column;

  JogError( const char* message, Ref<RefCounted> reader, int line, int column )
    : reader(reader), line(line), column(column)
  {
    this->message = new ASCIIString(message);
  }

  JogError( const char* message )
    : reader(NULL), line(0), column(0)
  {
    this->message = new ASCIIString(message);
  }

  JogError( Ref<ASCIIString> message, Ref<RefCounted> reader, int line, int column )
    : message(message), reader(reader), line(line), column(column)
  {
  }

  ~JogError()
  {
  }

  void print();

  Ref<ASCIIString> context();
};

int jog_char_to_value( int ch );
int jog_is_digit( int ch, int base );

struct JogReader : RefCounted
{
  Ref<ASCIIString> filename;
  int   line, column;

  short int* data;
  int   remaining;
  int   pos;

  int   original_size;

  JogReader( const char* filename );
  JogReader( const char* filename, const char* data, int size );
  JogReader( Ref<JogReader> existing );

  void init( const char* filename, const char* data, int size );

  ~JogReader();

  int peek();
  int peek( int num_ahead );
  int read();
  bool consume( int ch );
};


#define TOKEN_EOF            1
#define TOKEN_EOL            2
#define TOKEN_ID             3
#define TOKEN_LITERAL_CHAR   4
#define TOKEN_LITERAL_DOUBLE 5
#define TOKEN_LITERAL_FLOAT  6
#define TOKEN_LITERAL_LONG   7
#define TOKEN_LITERAL_INT    8
#define TOKEN_LITERAL_INT_V  9
#define TOKEN_LITERAL_LONG_V 10
  // INT_V  is -2^31
  // LONG_V is -2^63
  // These work around a special case overflow check.

#define TOKEN_STRING        11
#define TOKEN_UNKNOWN       12
#define TOKEN_BANG          13
#define TOKEN_PERCENT       14
#define TOKEN_AMPERSAND     15
#define TOKEN_LPAREN        16
#define TOKEN_RPAREN        17
#define TOKEN_STAR          18
#define TOKEN_PLUS          19
#define TOKEN_COMMA         20
#define TOKEN_MINUS         21
#define TOKEN_PERIOD        22
#define TOKEN_SLASH         23
#define TOKEN_COLON         24
#define TOKEN_SEMICOLON     25
#define TOKEN_LT            26
#define TOKEN_ASSIGN        27
#define TOKEN_GT            28
#define TOKEN_QUESTIONMARK  29
#define TOKEN_LBRACKET      30
#define TOKEN_BACKSLASH     31
#define TOKEN_RBRACKET      32
#define TOKEN_CARET         33
#define TOKEN_LCURLY        34
#define TOKEN_PIPE          35
#define TOKEN_RCURLY        36
#define TOKEN_TILDE         37

#define TOKEN_NE            38
#define TOKEN_MOD_ASSIGN    39
#define TOKEN_LOGICAL_AND   40
#define TOKEN_AND_ASSIGN    41
#define TOKEN_MUL_ASSIGN    42
#define TOKEN_ADD_ASSIGN    43
#define TOKEN_INCREMENT     44
#define TOKEN_DECREMENT     45
#define TOKEN_SUB_ASSIGN    46
#define TOKEN_DIV_ASSIGN    47

#define TOKEN_SHL           48
#define TOKEN_SHL_ASSIGN    49
#define TOKEN_LE            50
#define TOKEN_EQ            51
#define TOKEN_GE            52
#define TOKEN_SHRX          53
#define TOKEN_SHRX_ASSIGN   54
#define TOKEN_SHR           55
#define TOKEN_SHR_ASSIGN    56

#define TOKEN_XOR_ASSIGN    57
#define TOKEN_OR_ASSIGN     58
#define TOKEN_LOGICAL_OR    59

#define TOKEN_ABSTRACT      60
#define TOKEN_ASSERT        61
#define TOKEN_BREAK         62
#define TOKEN_CASE          63
#define TOKEN_CATCH         64
#define TOKEN_CLASS         65
#define TOKEN_CONST         66
#define TOKEN_CONTINUE      67
#define TOKEN_DEFAULT       68
#define TOKEN_DO            69
#define TOKEN_ELSE          70
#define TOKEN_ENUM          71
#define TOKEN_EXTENDS       72
#define TOKEN_FALSE         73
#define TOKEN_FINAL         74
#define TOKEN_FINALLY       75
#define TOKEN_FOR           76
#define TOKEN_GOTO          77
#define TOKEN_IF            78
#define TOKEN_IMPLEMENTS    79
#define TOKEN_IMPORT        80
#define TOKEN_INSTANCEOF    81
#define TOKEN_INTERFACE     82
#define TOKEN_NATIVE        83
#define TOKEN_NEW           84
#define TOKEN_NULL          85
#define TOKEN_PACKAGE       86
#define TOKEN_PRIVATE       87
#define TOKEN_PROTECTED     88
#define TOKEN_PUBLIC        89
#define TOKEN_RETURN        90
#define TOKEN_STATIC        91
#define TOKEN_STRICTFP      92
#define TOKEN_SUPER         93
#define TOKEN_SWITCH        94
#define TOKEN_SYNCHRONIZED  95
#define TOKEN_THROW         96
#define TOKEN_THROWS        97
#define TOKEN_TRANSIENT     98
#define TOKEN_TRUE          99
#define TOKEN_TRY           100
#define TOKEN_VOLATILE      101
#define TOKEN_WHILE         102

#define JOG_SHR( type, value, bits ) \
  (bits) ? ((value >> bits) & ((((type)1) << ((sizeof(type)*8)-bits)) - 1)) : value

struct StringComparator
{
  bool operator()( const char* a, const char* b )
  {
    return strcmp(a,b) < 0;
  }
};

typedef map<const char*,int,StringComparator> KeywordMap;

extern KeywordMap keywords;
extern const char* token_name_lookup[];

struct JogToken : RefCounted
{
  Ref<JogReader> reader;
  Ref<JogString> content;
  int line, column, type;

  JogToken( Ref<JogReader> reader, int line=0, int column=0 ) :
      reader(reader), line(line), column(column), type(0)
  {
  }

  ~JogToken()
  {
  }

  void print()
  {
    switch (type)
    {
      case TOKEN_ID:        printf( "%s\n", content->to_ascii()->data ); break;
      case TOKEN_LITERAL_CHAR: printf( "char(%s)\n", content->to_ascii()->data ); break;
      case TOKEN_LITERAL_DOUBLE:
      case TOKEN_LITERAL_FLOAT: 
      case TOKEN_LITERAL_LONG:  
      case TOKEN_LITERAL_INT:   
                            printf( "#%s\n", content->to_ascii()->data ); break;
      case TOKEN_LITERAL_INT_V:
                            printf( "#2^31\n" ); break;
      case TOKEN_LITERAL_LONG_V:
                            printf( "#2^63\n" ); break;
      case TOKEN_STRING:    printf( "\"%s\"\n", content->to_ascii()->data ); break;
      default:              printf( "%s\n", token_name_lookup[type] ); break;
    }
  }

  Ref<JogError> error( const char* message )
  {
    return new JogError(message,*reader,line,column); 
  }

  Ref<JogError> error( Ref<ASCIIString> message )
  {
    return new JogError(message,*reader,line,column); 
  }
};

struct JogScanner : RefCounted
{
  Ref<JogReader> reader;
  Ref<JogToken>  next;

  RefList<JogToken> pending_stack;
  RefList<JogToken> history_stack;
  ArrayList<int> marks;

  StringBuilder          buffer;
  UnicodeStringBuilder   unicode_buffer;

  JogScanner( Ref<JogReader> reader );

  void set_up_keywords();
  void consume_ws();
  bool has_another();
  Ref<JogToken> peek();
  Ref<JogToken> peek( int num_ahead );
  Ref<JogToken> read();
  void set_mark();
  void clear_mark();
  void rewind_to_mark();
  bool next_is( int token_type );
  bool next_is( const char* identifier );
  bool consume( int token_type );
  bool consume( const char* identifier );
  void must_consume( int token_type, const char* error_mesg );
  void must_consume_semicolon( Ref<JogToken> t );
  Ref<JogString> must_read_id( const char* error_mesg );
  void prep_next();
  void error( const char* mesg );
  bool consume_char( int ch );
  void must_consume_char( int ch );
  void scan_number();
  void scan_real();
  Ref<JogString> to_hex_string( long long int n, int bits );
  void scan_string();
  int scan_char();
};

//=============================================================================
//  JogCmd
//=============================================================================
struct JogTypeInfo;
struct JogVM;

struct JogCmd : RefCounted
{
  //static Ref<JogCmd> cmd_push_dummy_ref;

  virtual int node_type() { return __LINE__; }

  Ref<JogToken> t;

  JogCmd( Ref<JogToken> t ) : t(t) { }

  virtual void print() = 0;

  virtual bool is_literal() { return false; }
  virtual bool is_literal_int() { return false; }

  virtual JogTypeInfo* reinterpret_as_type() { return NULL; }
  JogTypeInfo* as_type();

  virtual JogTypeInfo* type() = 0;
  JogTypeInfo* require_value();

  Ref<JogCmd>  discarding_result();

  virtual Ref<JogCmd> resolve( Ref<JogCmd> context )
  {
    throw t->error( "[Internal] Resolve-in-context not supported by this node." );
  }

  virtual Ref<JogCmd> resolve( JogTypeInfo* class_context, Ref<JogCmd> context )
  {
    throw t->error( "No such class member." );
  }

  virtual Ref<JogCmd> resolve_assignment( Ref<JogCmd> context, Ref<JogCmd> new_value )
  {
    throw t->error( "Illegal assignment." );
  }

  virtual Ref<JogCmd> resolve_assignment( JogTypeInfo* class_context, Ref<JogCmd> context, 
      Ref<JogCmd> new_value )
  {
    throw t->error( "Illegal assignment." );
  }

  virtual Ref<JogCmd> resolve_op_assign( int op_type, Ref<JogCmd> context, Ref<JogCmd> rhs )
  {
    throw t->error( "Illegal assignment." );
  }

  virtual Ref<JogCmd> resolve_stepcount_access( int when, int modifier )
  {
    throw t->error( "++/-- operand must be a single variable." );
  }

  virtual Ref<JogCmd> resolve_stepcount_access( int when, int modifier,
      JogTypeInfo* class_context, Ref<JogCmd> context )
  {
    throw t->error( "++/-- operand must be a single variable." );
  }

  virtual Ref<JogCmd> resolve_stepcount_access( int when, int modifier, Ref<JogCmd> context )
  {
    throw t->error( "++/-- operand must be a single variable." );
  }

  virtual Ref<JogCmd> resolve();
  virtual Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  virtual void on_push( JogVM* vm );

  virtual void execute( JogVM* vm );

  void require_boolean();
  JogTypeInfo* require_integer();
  JogTypeInfo* require_integer_or_boolean();

  JogTypeInfo* require_reference();
  JogTypeInfo* require_instance_of( JogTypeInfo* base_type );
  JogTypeInfo* require_primitive();

  Ref<JogError> error( const char* mesg ) { return t->error(mesg); } 

  JogInt64 zero_check( JogInt64 value )
  {
    if (value == 0) throw t->error( "Illegal division by zero." );
    return value;
  }

  JogInt32 zero_check( JogInt32 value )
  {
    if (value == 0) throw t->error( "Illegal division by zero." );
    return value;
  }

  JogInt16 zero_check( JogInt16 value )
  {
    if (value == 0) throw t->error( "Illegal division by zero." );
    return value;
  }

  JogInt8 zero_check( JogInt8 value )
  {
    if (value == 0) throw t->error( "Illegal division by zero." );
    return value;
  }

  JogChar zero_check( JogChar value )
  {
    if (value == 0) throw t->error( "Illegal division by zero." );
    return value;
  }

  JogMethodInfo* resolve_call( Ref<JogToken> t, JogTypeInfo* context_type,
      Ref<JogString> name, Ref<JogCmd> args, bool allow_object_methods=true );
};

//=============================================================================
//  JogInstruction
//=============================================================================
struct JogInstruction
{
  JogCmd* command;
  int     state;  // command-specific state

  JogInstruction() : command(NULL), state(0)
  {
  }

  JogInstruction( JogCmd* cmd ) : command(cmd), state(0)
  {
  }

  JogInstruction( JogCmd* cmd, int state ) : command(cmd), state(state)
  {
  }
};


//=============================================================================
//  JogObject, JogRef
//=============================================================================
struct JogObject
{
  int          reference_count;
  JogObject*   next_object;
  JogTypeInfo* type;
  JogInt64     data[1];  // may actually be any size

  // Note: all object data is externally memset to 0 when it is declared.

  ~JogObject();

  inline void retain() 
  { 
    ++reference_count;
  }

  inline void release()
  {
    if ( !--reference_count ) release_refs();
  }

  void release_refs();
  int  total_object_bytes();
};


struct JogRef
{
  JogObject* object;

  JogRef() : object(NULL) { }

  JogRef( JogObject* object ) : object(object)
  {
    if (object) object->retain();
  }

  JogRef( const JogRef& existing )
  {
    object = NULL;
    operator=(existing);
  }

  ~JogRef()
  {
    if (object) 
    {
      object->release();
      object = NULL;
    }
  }

  JogObject* null_check( Ref<JogToken> t )
  {
    if (object == NULL) throw t->error( "Null Pointer Exception." );
    return object;
  }

  void operator=( const JogRef& other )
  {
    if (other.object) other.object->retain();
    if (object) object->release();
    object = other.object;
  }

  void operator=( JogObject* new_object )
  {
    if (object) object->release();
    object = new_object;
    if (object) object->retain();
  }

  JogObject* operator*()
  {
    return (JogObject*) object;
  }

  JogObject* operator->()
  {
    return (JogObject*) object;
  }

  bool operator==( void* other )
  {
    return object == other;
  }

  bool operator==( JogRef other )
  {
    return object == other.object;
  }

  bool operator!=( void* other )
  {
    return object != other;
  }

  bool operator!=( JogRef other )
  {
    return object != other.object;
  }

  bool operator!()
  {
    return object ? false : true;
  }
};


//=============================================================================
//  JogStackFrame
//=============================================================================
struct JogStackFrame
{
  JogInstruction* instruction_stack_ptr;
  JogInt64*       data_stack_ptr;
  JogRef*         ref_stack_ptr;
  JogMethodInfo*  called_method;

  JogStackFrame()
  {
  }

  void init( JogInstruction* instruction, JogInt64* data, JogRef* ref, 
      JogMethodInfo* called_method )
  {
    instruction_stack_ptr = instruction;
    data_stack_ptr = data;
    ref_stack_ptr = ref;
    this->called_method = called_method;
  }
};

//=============================================================================
//  JogVM
//=============================================================================
#define JOG_INSTRUCTION_STACK_CAPACITY 1024
#define JOG_DATA_STACK_CAPACITY        8192
#define JOG_REF_STACK_CAPACITY         8192
#define JOG_FRAME_STACK_CAPACITY       1024

typedef void (*JogNativeMethodHandler)(JogVM*);

class JogNativeMethodLookup : public map<Ref<JogString>,JogNativeMethodHandler,JogStringComparator>
{
  public:
    bool contains( Ref<JogString> sig )
    {
      return (map<Ref<JogString>,JogNativeMethodHandler,JogStringComparator>::find(sig) != end());
    }

    JogNativeMethodHandler find( Ref<JogString> sig )
    {
      JogNativeMethodLookup::iterator entry = map<Ref<JogString>,JogNativeMethodHandler,JogStringComparator>::find(sig);
      if (entry == end()) return NULL;
      return entry->second;
    }
};

struct JogVM : RefCounted
{
  int max_object_bytes;  // set as desired

  int cur_object_bytes;  // internal

  JogObject* all_objects;

  JogInstruction     instruction_stack[JOG_INSTRUCTION_STACK_CAPACITY];
  JogInstruction*    instruction_stack_ptr;
  JogInstruction*    instruction_stack_limit;

  JogInt64           data_stack[JOG_DATA_STACK_CAPACITY];
  JogInt64*          data_stack_ptr;
  JogInt64*          data_stack_limit;

  JogRef             ref_stack[JOG_REF_STACK_CAPACITY];
  JogRef*            ref_stack_ptr;
  JogRef*            ref_stack_limit;

  JogStackFrame      frames[JOG_FRAME_STACK_CAPACITY];
  JogStackFrame*     frame_ptr;

  JogNativeMethodLookup native_methods;

  ArrayList<JogTypeInfo*> types;

  JogVM() : max_object_bytes(1024*1024), cur_object_bytes(0), all_objects(NULL)
  {
    instruction_stack_limit = instruction_stack + JOG_INSTRUCTION_STACK_CAPACITY;
    instruction_stack_ptr   = instruction_stack_limit;
    data_stack_limit        = data_stack + JOG_DATA_STACK_CAPACITY;
    data_stack_ptr          = data_stack_limit;
    ref_stack_limit         = ref_stack + JOG_REF_STACK_CAPACITY;
    ref_stack_ptr           = ref_stack_limit;
    frame_ptr               = frames + JOG_FRAME_STACK_CAPACITY;
  }

  ~JogVM() { reset(); }

  void reset()
  {
    delete_all_objects();
  }

  void parse( string filename );
  void compile();
  void run( const char* main_class_name );

  void add_native_handler( const char* signature, JogNativeMethodHandler handler )
  {
    add_native_handler( new JogString(signature), handler );
  }

  void add_native_handler( Ref<JogString> signature, JogNativeMethodHandler handler );

  void force_garbage_collection();
  void delete_all_objects();
  void register_object( JogObject* obj, int byte_size );

  void push_frame( JogMethodInfo* method_info );

  void pop_frame() 
  {
    instruction_stack_ptr = frame_ptr->instruction_stack_ptr;
    data_stack_ptr = frame_ptr->data_stack_ptr;

    JogRef* target_ptr = frame_ptr->ref_stack_ptr;
    JogRef* cur_ptr    = ref_stack_ptr;
    while (cur_ptr != target_ptr) *(cur_ptr++) = NULL;
    ref_stack_ptr = cur_ptr;

    ++frame_ptr; 
  }

  void push( JogCmd* cmd )
  {
    if (instruction_stack_ptr == instruction_stack)
    {
      throw cmd->error("Instruction stack limit reached during recursion.");
    }
    *(--instruction_stack_ptr) = JogInstruction(cmd);
    cmd->on_push(this);
  }

  void push( JogCmd* cmd, int execution_state )
  {
    if (instruction_stack_ptr == instruction_stack)
    {
      throw cmd->error("Instruction stack limit reached during recursion.");
    }
    *(--instruction_stack_ptr) = JogInstruction(cmd,execution_state);
  }

  void push( JogInt64 value )
  {
    if (data_stack_ptr == data_stack)
    {
      Ref<JogError> err = new JogError("Data stack limit reached during recursion.");
      throw err;
    }
    *(--data_stack_ptr) = value;
  }

  void push( int value )
  {
    if (data_stack_ptr == data_stack)
    {
      Ref<JogError> err = new JogError("Data stack limit reached during recursion.");
      throw err;
    }
    *(--data_stack_ptr) = value;
  }

  void push( double value )
  {
    if (data_stack_ptr == data_stack)
    {
      Ref<JogError> err = new JogError("Data stack limit reached during recursion.");
      throw err;
    }
    *(--data_stack_ptr) = *((JogInt64*)&value);
  }

  void push( JogRef object )
  {
    if (ref_stack_ptr == ref_stack)
    {
      Ref<JogError> err = new JogError("Reference stack limit reached during recursion.");
      throw err;
    }
    *(--ref_stack_ptr) = object;
  }

  JogInt64 pop_data()
  {
    if (data_stack_ptr == data_stack_limit)
    {
      Ref<JogError> err = new JogError("[Internal] Data stack underflow.");
      throw err;
    }
    return *(data_stack_ptr++);
  }

  double pop_data_as_Real64()
  {
    if (data_stack_ptr == data_stack_limit)
    {
      Ref<JogError> err = new JogError("[Internal] Data stack underflow.");
      throw err;
    }
    JogInt64 result = *(data_stack_ptr++);
    return *((double*)&result);
  }

  JogInt64 pop_long()
  {
    if (data_stack_ptr == data_stack_limit)
    {
      Ref<JogError> err = new JogError("[Internal] Data stack underflow.");
      throw err;
    }
    return *(data_stack_ptr++);
  }

  int pop_int()
  {
    if (data_stack_ptr == data_stack_limit)
    {
      Ref<JogError> err = new JogError("[Internal] Data stack underflow.");
      throw err;
    }
    return (int) *(data_stack_ptr++);
  }

  double pop_double()
  {
    if (data_stack_ptr == data_stack_limit)
    {
      Ref<JogError> err = new JogError("[Internal] Data stack underflow.");
      throw err;
    }
    JogInt64 result = *(data_stack_ptr++);
    return *((double*)&result);
  }

  JogRef pop_ref()
  {
    if (ref_stack_ptr == ref_stack_limit)
    {
      Ref<JogError> err = new JogError("[Internal] Ref stack underflow.");
      throw err;
    }
    JogRef result = *ref_stack_ptr;
    *(ref_stack_ptr++) = NULL;
    return result;
  }

  JogInt64 peek_data()
  {
    return *data_stack_ptr;
  }

  double peek_data_as_Real64()
  {
    JogInt64 result = *data_stack_ptr;
    return *((double*)&result);
  }

  JogRef peek_ref()
  {
    return *ref_stack_ptr;
  }


  int  execution_state()
  {
    // Returns and increments the execution state of the current
    // instruction.  0 will be the first value returned.
    return instruction_stack_ptr[-1].state++;
  }

  void run_this_again()
  {
    // The current instruction was just popped off the stack, so
    // bumping up the stack position will run it again.
    // CAUTION: run_this_again() must be called before any new
    // commands are pushed on the stack.
    --instruction_stack_ptr;
  }

  void execute()
  {
    while (instruction_stack_ptr != instruction_stack_limit)
    {
      //printf( "line %d\n", (*(instruction_stack_ptr)).command->node_type() );
      (*(instruction_stack_ptr++)).command->execute(this);
    }
  }

  void call_native( JogMethodInfo* m );
    // internal use

  void call_void_method( JogRef context, const char* signature );
};


//=============================================================================
//  JogCmdList
//=============================================================================
struct JogCmdList : JogCmd
{
  int node_type() { return __LINE__; }

  RefList<JogCmd> commands;

  JogCmdList( Ref<JogToken> t ) : JogCmd(t)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    for (int i=0; i<commands.count; ++i)
    {
      printf("  ");
      commands[i]->print();
      printf("\n");
    }
  }

  void add( Ref<JogCmd> cmd )
  {
    commands.add( cmd );
  }

  int count() { return commands.count; }

  Ref<JogCmd> operator[]( int index )
  {
    return commands[index];
  }

  Ref<JogCmd> resolve();

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogStatementList : JogCmdList
{
  int node_type() { return __LINE__; }

  JogStatementList( Ref<JogToken> t ) : JogCmdList(t)
  {
  }

  Ref<JogCmd> resolve();
};

#define JOG_QUALIFIER_PUBLIC        1
#define JOG_QUALIFIER_PROTECTED     2
#define JOG_QUALIFIER_PRIVATE       4
#define JOG_QUALIFIER_STATIC        8
#define JOG_QUALIFIER_NATIVE       16
#define JOG_QUALIFIER_CLASS        32
#define JOG_QUALIFIER_PRIMITIVE    64
#define JOG_QUALIFIER_CONSTRUCTOR 128

#define JOG_QUALIFIER_REFERENCE  (JOG_QUALIFIER_CLASS)


//=============================================================================
//  JogPropertyInfo
//=============================================================================
struct JogPropertyInfo : RefCounted
{
  Ref<JogToken>  t;
  int qualifiers;
  int index;    // in object data
  JogTypeInfo*   type_context;
  JogTypeInfo*   type;
  Ref<JogString> name;
  Ref<JogCmd>    initial_value;

  JogPropertyInfo( Ref<JogToken> t, int qualifiers, JogTypeInfo* type_context, 
      JogTypeInfo* type, Ref<JogString> name, Ref<JogCmd> initial_value );

  bool is_static() { return (qualifiers & JOG_QUALIFIER_STATIC) != 0; }

  void print();

  /*
  JogPropertyInfo* duplicate()
  {
    JogPropertyInfo* result = new JogPropertyInfo( t, qualifiers, type, name, NULL );
    result->index = index;
    return result;
  }
  */
};


//=============================================================================
//  JogLocalVarInfo
//=============================================================================
struct JogLocalVarInfo : RefCounted
{
  Ref<JogToken>  t;
  JogTypeInfo*   type;
  Ref<JogString> name;
  int            index;
  int            offset;  // relative to stack frame

  JogLocalVarInfo( Ref<JogToken> t, JogTypeInfo* type, Ref<JogString> name );
  void print();
};


//=============================================================================
//  JogMethodInfo
//=============================================================================
struct JogMethodInfo : RefCounted
{
  static int next_method_id;

  Ref<JogToken>   t;
  int             qualifiers;
  JogTypeInfo*    type_context;
  JogTypeInfo*    return_type;
  bool            calls_super_constructor;

  Ref<JogString>  name;
  Ref<JogString>  signature;
  Ref<JogString>  full_signature;
  RefList<JogLocalVarInfo> parameters;
  RefList<JogLocalVarInfo> locals;  // Includes parameters
  Ref<JogStatementList> statements;

  int  method_id;
  int  dispatch_id;
  JogNativeMethodHandler native_handler;

  int  param_data_count;
  int  param_ref_count;   // includes object context

  int  local_data_count;
  int  local_ref_count;

  bool organized;
  bool resolved;

  JogMethodInfo( Ref<JogToken> t, int qualifiers, JogTypeInfo* type_context,
      JogTypeInfo* return_type, Ref<JogString> name );

  void print();

  bool is_static() { return (qualifiers & JOG_QUALIFIER_STATIC) != 0; }
  bool is_native() { return (qualifiers & JOG_QUALIFIER_NATIVE) != 0; }
  bool is_constructor() { return (qualifiers & JOG_QUALIFIER_CONSTRUCTOR) != 0; }

  bool is_less_specific_than( JogMethodInfo* other );
  bool is_inherited( JogTypeInfo* by_type );

  void organize();
  void resolve();
};



//=============================================================================
//  JogTypeInfo
//=============================================================================
class JogPropertyLookup : public map<Ref<JogString>,JogPropertyInfo*,JogStringComparator>
{
  public:
    bool contains( Ref<JogString> name )
    {
      return ((*this)[name] != NULL);
    }
};

class JogDispatchIDLookup : public map<Ref<JogString>,int,JogStringComparator>
{
  public:
    int  next_id;

    JogDispatchIDLookup() : next_id(1) { }

    int& operator[]( Ref<JogString> sig )
    {
      int& result = map<Ref<JogString>,int,JogStringComparator>::operator[](sig);
      if (result == 0)
      {
        result = next_id++;
      }
      return result;
    }
};

class JogMethodLookup : public map<Ref<JogString>,JogMethodInfo*,JogStringComparator>
{
  public:
    bool contains( Ref<JogString> signature )
    {
      return (find(signature) != end());
    }

    bool contains( const char* st )
    {
      Ref<JogString> signature = new JogString(st);
      return contains( signature );
    }

    JogMethodInfo*& operator[]( Ref<JogString> st )
    {
      return map<Ref<JogString>,JogMethodInfo*,JogStringComparator>::operator[](st);
    }

    JogMethodInfo*& operator[]( const char* st )
    {
      Ref<JogString> signature = new JogString(st);
      return operator[](signature);
    }
};

class JogMethodSet : public map<Ref<JogString>,ArrayList<JogMethodInfo*>*,JogStringComparator>
{
  public:
    bool contains( Ref<JogString> name )
    {
      return (find(name) != end());
    }
};

struct JogTypeInfo : RefCounted
{
  int qualifiers;        // Bitwise combination of JOG_QUALIFIER_X

  // in bytes
  int class_data_count;
  int data_count;
  int object_size;

  JogInt64* class_data;

  Ref<JogToken>             t;
  Ref<JogString>            name;
  RefList<JogPropertyInfo>  class_properties;
  RefList<JogMethodInfo>    class_methods;
  RefList<JogPropertyInfo>  properties;
  RefList<JogMethodInfo>    methods;
  RefList<JogMethodInfo>    static_initializers;
  Ref<JogMethodInfo>        m_init_object;
  Ref<JogCmd>               call_init_object;
  ArrayList<JogMethodInfo*> dispatch_table;

  JogTypeInfo*            element_type; // Only non-NULL for arrays.
  JogTypeInfo*            base_class;   // set to NULL for Object, set to Object for aspects
  ArrayList<JogTypeInfo*> interfaces;

  JogPropertyLookup class_properties_by_name;
  JogPropertyLookup properties_by_name;

  JogMethodSet    class_methods_by_name;
  JogMethodSet    methods_by_name;
  JogMethodLookup class_methods_by_signature;
  JogMethodLookup methods_by_signature;

  bool organized;
  bool prepped;
  bool resolved;

  static JogTypeInfo* create( Ref<JogToken> t, int qualifiers, Ref<JogString> name );
  static JogTypeInfo* create( Ref<JogToken> t, int qualifiers, const char* name );
  static JogTypeInfo* reference( Ref<JogToken> t, Ref<JogString> name );
  static JogTypeInfo* find( Ref<JogString> name );
  static JogTypeInfo* find( const char* name ) { return find(new JogString(name)); }

  JogTypeInfo() : 
    class_data(NULL),
    element_type(NULL),
    base_class(NULL),
    organized(false), prepped(false), resolved(false)
  {
  }

  ~JogTypeInfo()
  {
    if (class_data)
    {
      delete class_data;
      class_data = NULL;
    }
  }

  JogRef create_instance( JogVM* vm )
  {
    JogObject* obj = (JogObject*) new char[object_size];
    memset( obj, 0, object_size );
    obj->type = this;

    JogRef result(obj);
    vm->register_object(obj,object_size);

    return result;
  }

  bool is_type() { return (qualifiers != 0); }
  bool is_array() { return element_type != NULL; }

  bool instance_of( JogTypeInfo* base_type );

  bool is_primitive() { return (qualifiers & JOG_QUALIFIER_PRIMITIVE) != 0; }
  bool is_reference() { return (qualifiers & JOG_QUALIFIER_REFERENCE) != 0; }

  bool is_boolean();
  bool is_real();
  bool is_integer();

  int  precision();

  bool is_compatible_with( JogTypeInfo* other );

  void print()
  {
    name->print();
  }

  void print_members()
  {
    name->print();
    printf("\n");
    for (int i=0; i<properties.count; ++i) properties[i]->print();
    for (int i=0; i<methods.count; ++i) methods[i]->print();
  }

  void organize();
  void add( Ref<JogMethodInfo> m );

  void prep();
  void resolve();

};


//=============================================================================
//  JogTypeManager
//=============================================================================
class JogTypeLookup : public map<Ref<JogString>,Ref<JogTypeInfo>,JogStringComparator>
{
  public:
  bool contains( Ref<JogString> name )
  {
    return (find(name) != end());
  }
};

struct JogTypeManager
{
  JogTypeLookup type_lookup;
  JogDispatchIDLookup dispatch_id_lookup;

  JogTypeInfo* type_void;
  JogTypeInfo* type_real64;
  JogTypeInfo* type_real32;
  JogTypeInfo* type_int64;
  JogTypeInfo* type_int32;
  JogTypeInfo* type_int16;
  JogTypeInfo* type_int8;
  JogTypeInfo* type_char;
  JogTypeInfo* type_boolean;
  JogTypeInfo* type_object;
  JogTypeInfo* type_null;
  JogTypeInfo* type_string;

  JogTypeManager()
  {
    Ref<JogReader> reader = new JogReader( "[INTERNAL]", NULL, 0 );
    Ref<JogToken>  t = new JogToken( reader, 0, 0 );
    int quals = JOG_QUALIFIER_PRIMITIVE;
    type_void    = JogTypeInfo::create( t, quals, "void" );
    type_real64  = JogTypeInfo::create( t, quals, "double" );
    type_real32  = JogTypeInfo::create( t, quals, "float" );
    type_int64   = JogTypeInfo::create( t, quals, "long" );
    type_int32   = JogTypeInfo::create( t, quals, "int" );
    type_int16   = JogTypeInfo::create( t, quals, "short" );
    type_int8    = JogTypeInfo::create( t, quals, "byte" );
    type_char    = JogTypeInfo::create( t, quals, "char" );
    type_boolean = JogTypeInfo::create( t, quals, "boolean" );
    type_null    = JogTypeInfo::create( t, JOG_QUALIFIER_CLASS|JOG_QUALIFIER_PUBLIC, "null" );

    type_real64->organized  = true;
    type_real32->organized  = true;
    type_int64->organized   = true;
    type_int32->organized   = true;
    type_int16->organized   = true;
    type_int8->organized    = true;
    type_char->organized    = true;
    type_boolean->organized = true;
  }

  JogTypeInfo* must_find_type( const char* name );
  JogTypeInfo* find_common_type( JogToken* t, JogCmd* cmd1, JogCmd* cmd2, bool min32=false );
};

extern JogTypeManager jog_type_manager;


struct JogCmdLiteralReal64 : JogCmd
{
  int node_type() { return __LINE__; }

  double value;

  JogCmdLiteralReal64( Ref<JogToken> t, double value ) : JogCmd(t), value(value) { }

  JogCmdLiteralReal64( Ref<JogToken> t, Ref<JogString> hex_string ) : JogCmd(t)
  {
    long long int n = hex_string->to_int64();
    value = *((double*)&n);
  }

  JogTypeInfo* type() { return jog_type_manager.type_real64; }

  void print()
  {
    printf("%.4lf",value);
  }

  bool is_literal() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdLiteralReal32 : JogCmd
{
  int node_type() { return __LINE__; }

  float value;

  JogCmdLiteralReal32( Ref<JogToken> t, float value ) : JogCmd(t), value(value) { }

  JogCmdLiteralReal32( Ref<JogToken> t, Ref<JogString> hex_string ) : JogCmd(t)
  {
    JogInt32 n = (JogInt32) hex_string->to_int64();
    value = *((float*)&n);
  }

  JogTypeInfo* type() { return jog_type_manager.type_real32; }

  void print()
  {
    printf("%.4ff",value);
  }

  bool is_literal() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm ) ;
};

struct JogCmdLiteralInt64 : JogCmd
{
  int node_type() { return __LINE__; }

  JogInt64 value;

  JogCmdLiteralInt64( Ref<JogToken> t, JogInt64 value ) : JogCmd(t), value(value) { }

  JogCmdLiteralInt64( Ref<JogToken> t, Ref<JogString> hex_string ) : JogCmd(t)
  {
    value = hex_string->to_int64();
  }

  JogTypeInfo* type() { return jog_type_manager.type_int64; }

  void print()
  {
    printf("%lldL",value);
  }

  bool is_literal() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdLiteralInt32 : JogCmd
{
  int node_type() { return __LINE__; }

  int value;

  JogCmdLiteralInt32( Ref<JogToken> t, int value ) : JogCmd(t), value(value) { }

  JogCmdLiteralInt32( Ref<JogToken> t, Ref<JogString> hex_string ) : JogCmd(t)
  {
    value = int(hex_string->to_int64());
  }

  JogTypeInfo* type() { return jog_type_manager.type_int32; }

  void print()
  {
    printf("%d",value);
  }

  bool is_literal() { return true; }
  bool is_literal_int() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdLiteralInt16 : JogCmd
{
  int node_type() { return __LINE__; }

  JogInt16 value;

  JogCmdLiteralInt16( Ref<JogToken> t, int value ) : JogCmd(t), value(value) { }

  JogCmdLiteralInt16( Ref<JogToken> t, Ref<JogString> hex_string ) : JogCmd(t)
  {
    value = JogInt16(hex_string->to_int64());
  }

  JogTypeInfo* type() { return jog_type_manager.type_int16; }

  void print()
  {
    printf("%dS",(int)value);
  }

  bool is_literal() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdLiteralInt8 : JogCmd
{
  int node_type() { return __LINE__; }

  JogInt8 value;

  JogCmdLiteralInt8( Ref<JogToken> t, int value ) : JogCmd(t), value(value) { }

  JogCmdLiteralInt8( Ref<JogToken> t, Ref<JogString> hex_string ) : JogCmd(t)
  {
    value = JogInt8(hex_string->to_int64());
  }

  JogTypeInfo* type() { return jog_type_manager.type_int8; }

  void print()
  {
    printf("%dB",value);
  }

  bool is_literal() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdLiteralChar : JogCmd
{
  int node_type() { return __LINE__; }

  int value;

  JogCmdLiteralChar( Ref<JogToken> t, int value ) : JogCmd(t), value(value) { }

  JogCmdLiteralChar( Ref<JogToken> t, Ref<JogString> hex_string ) : JogCmd(t)
  {
    value = JogChar(hex_string->to_int64());
  }

  JogTypeInfo* type() { return jog_type_manager.type_char; }

  void print()
  {
    if (value >= 32 && value <= 126) printf("'%c'",value);
    else printf("'%d'",value);
  }

  bool is_literal() { return true; }
  bool is_literal_int() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdLiteralBoolean : JogCmd
{
  int node_type() { return __LINE__; }

  bool value;

  JogCmdLiteralBoolean( Ref<JogToken> t, bool value ) : JogCmd(t), value(value) { }

  JogTypeInfo* type() { return jog_type_manager.type_boolean; }

  void print()
  {
    printf( value ? "true" : "false" );
  }

  bool is_literal() { return true; }

  Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdLiteralString : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogString> value;

  JogCmdLiteralString( Ref<JogToken> t, Ref<JogString> value ) : JogCmd(t), value(value) { }

  JogTypeInfo* type() { return jog_type_manager.type_string; }

  void print()
  {
    printf("\"");
    value->print();
    printf("\"");
  }

  //Ref<JogCmd> cast_to_type( JogTypeInfo* to_type );
};

struct JogCmdArgs : JogCmdList
{
  int node_type() { return __LINE__; }

  JogCmdArgs( Ref<JogToken> t ) : JogCmdList(t)
  {
  }

  void print()
  {
    printf("(");
    if (commands.count > 0)
    {
      for (int i=0; i<commands.count; ++i)
      {
        if (i > 0 )printf(",");
        commands[i]->print();
      }
    }
    printf(")");
  }
};

struct JogCmdLocalVarDeclarations : JogCmdList
{
  int node_type() { return __LINE__; }

  JogCmdLocalVarDeclarations( Ref<JogToken> t ) : JogCmdList(t)
  {
  }

  void print()
  {
    for (int i=0; i<commands.count; ++i)
    {
      commands[i]->print();
      printf("\n");
    }
  }
};

struct JogCmdBlock : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogStatementList> statements;

  JogCmdBlock( Ref<JogToken> t ) : JogCmd(t)
  {
    statements = new JogStatementList(t);
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    printf("{\n");
    statements->print();
    printf("}");
  }

  Ref<JogCmd> resolve();

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdControlStructure : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> body;

  JogCmdControlStructure( Ref<JogToken> t ) : JogCmd(t)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    body->print();
  }
};


struct JogCmdIf : JogCmdControlStructure
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> expression;
  Ref<JogCmd> else_body;

  JogCmdIf( Ref<JogToken> t, Ref<JogCmd> expression ) 
    : JogCmdControlStructure(t), expression(expression)
  {
  }

  Ref<JogCmd> resolve();

  void print()
  {
    printf("if (");
    expression->print();
    printf(")\n");
    if (*body) body->print();
    if (*else_body)
    {
      printf("else");
      else_body->print();
    }
  }

  void on_push( JogVM* vm  );

  void execute( JogVM* vm ) ;
};


struct JogCmdWhile : JogCmdControlStructure
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> expression;

  JogCmdWhile( Ref<JogToken> t, Ref<JogCmd> expression ) 
    : JogCmdControlStructure(t), expression(expression)
  {
  }

  void print()
  {
    printf("while (");
    expression->print();
    printf(")\n");
    JogCmdControlStructure::print();
  }

  Ref<JogCmd> resolve();

  void on_push( JogVM* vm );

  void execute( JogVM* vm ) ;
};

struct JogCmdFor : JogCmdControlStructure
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> initialization;
  Ref<JogCmd> condition;
  Ref<JogCmd> var_mod;

  JogCmdFor( Ref<JogToken> t, 
      Ref<JogCmd> initialization, Ref<JogCmd> condition, Ref<JogCmd> var_mod ) 
    : JogCmdControlStructure(t), 
      initialization(initialization), condition(condition), var_mod(var_mod)
  {
  }

  void print()
  {
    printf("for (");
    initialization->print();
    printf(";");
    condition->print();
    printf(";");
    var_mod->print();
    printf(")\n");
    JogCmdControlStructure::print();
  }

  Ref<JogCmd> resolve();

  void on_push( JogVM* vm );

  void execute( JogVM* vm ) ;
};

struct JogCmdUnary : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> operand;

  JogCmdUnary( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmd(t), operand(operand)
  {
  }

  JogTypeInfo* type() { return operand->type(); }

  void on_push( JogVM* vm );
};


struct JogCmdCast : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogTypeInfo* to_type;

  JogCmdCast( Ref<JogToken> t, Ref<JogCmd> operand, JogTypeInfo* to_type )
    : JogCmdUnary(t,operand), to_type(to_type)
  {
  }

  JogTypeInfo* type() { return to_type; }

  void print()
  {
    printf( "(" );
    to_type->print();
    printf( ")" );
    operand->print();
  }

  Ref<JogCmd> cast_to_type( JogTypeInfo* new_type )
  {
    if (new_type == to_type) return this;  // already cast
    return JogCmd::cast_to_type(new_type);
  }

  Ref<JogCmd> resolve();
};


struct JogCmdAssign : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> location, new_value;

  JogCmdAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> new_value ) 
    : JogCmd(t), location(location), new_value(new_value)
  {
    /*
    JogTypeInfo* storage_type = location->require_value();
    JogTypeInfo* value_type   = new_value->require_value();

    if (storage_type->is_primitive() != value_type->is_primitive())
    {
      if (value_type->is_primitive())
      {
        throw error( "Cannot assign a primitive value to an object reference." );
      }
      else
      {
        throw error( "Cannot assign an object reference to a primitive value." );
      }
    }

    if (storage_type->is_boolean() ^ value_type->is_boolean())
    {
      if (value_type->is_boolean())
      {
        throw error( "Cannot assign a boolean to a non-boolean value." );
      }
      else
      {
        throw error( "Cannot assign a non-boolean to a boolean value." );
      }
    }

    if (value_type->precision() > storage_type->precision())
    {
      throw error( "The type of the value being assigned has greater precision than the variable it is being assigned to.  Add a type cast (e.g. \"int i = x;\" -> \"int i = (int) x;\") to explicitly allow this loss of precision." );
    }

    this->new_value = new_value->cast_to_type( storage_type );
    */
  }

  JogTypeInfo* type() { return location->type(); }

  void print()
  {
    printf( "(" );
    location->print();
    printf( "=" );
    new_value->print();
    printf( ")" );
  }

  Ref<JogCmd> resolve();
};

struct JogCmdOpAssign : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> location, rhs;

  JogCmdOpAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmd(t), location(location), rhs(rhs)
  {
    /*
    JogTypeInfo* storage_type = location->require_value();
    JogTypeInfo* value_type   = rhs->require_value();

    if (storage_type->is_primitive() != value_type->is_primitive())
    {
      throw error( "Illegal operation between an object reference and a primitive." );
    }

    if (storage_type->is_boolean() ^ value_type->is_boolean())
    {
      throw error( "Illegal operation between a boolean and a non-boolean value." );
    }

    this->rhs = rhs->cast_to_type( storage_type );
    */
  }

  JogTypeInfo* type() { return location->type(); }

  virtual const char* symbol() { return "???"; }

  void print()
  {
    printf( "(" );
    location->print();
    printf( "%s", symbol() );
    rhs->print();
    printf( ")" );
  }

};

struct JogCmdAddAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdAddAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "+="; }

  Ref<JogCmd> resolve();
};

struct JogCmdSubAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdSubAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "-="; }

  Ref<JogCmd> resolve();
};

struct JogCmdMulAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdMulAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "*="; }

  Ref<JogCmd> resolve();
};

struct JogCmdDivAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }


  JogCmdDivAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "/="; }

  Ref<JogCmd> resolve();
};

struct JogCmdModAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdModAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "%%="; }

  Ref<JogCmd> resolve();
};

struct JogCmdAndAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdAndAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "&="; }

  Ref<JogCmd> resolve();
};

struct JogCmdOrAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdOrAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "|="; }

  Ref<JogCmd> resolve();
};

struct JogCmdXorAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdXorAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "^="; }

  Ref<JogCmd> resolve();
};

struct JogCmdLeftShiftAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdLeftShiftAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return "<<="; }

  Ref<JogCmd> resolve();
};

struct JogCmdRightXShiftAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdRightXShiftAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return ">>="; }

  Ref<JogCmd> resolve();
};

struct JogCmdRightShiftAssign : JogCmdOpAssign
{
  int node_type() { return __LINE__; }

  JogCmdRightShiftAssign( Ref<JogToken> t, Ref<JogCmd> location, Ref<JogCmd> rhs ) 
    : JogCmdOpAssign( t, location, rhs )
  {
  }

  const char* symbol() { return ">>>="; }

  Ref<JogCmd> resolve();
};

struct JogCmdConditional : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> condition, true_value, false_value;

  JogCmdConditional( Ref<JogToken> t, Ref<JogCmd> condition, 
      Ref<JogCmd> true_value, Ref<JogCmd> false_value ) 
    : JogCmd(t), condition(condition), true_value(true_value), false_value(false_value)
  { 
    /*
    condition->require_boolean();

    JogTypeInfo* common_type = jog_type_manager.find_common_type( *t, *true_value, *false_value );
    this->true_value = true_value->cast_to_type( common_type );
    this->false_value = false_value->cast_to_type( common_type );
    */
  }

  JogTypeInfo* type() { return true_value->type(); }

  void print()
  {
    printf("(");
    condition->print();
    printf("?");
    true_value->print();
    printf(":");
    false_value->print();
    printf(")");
  }
};

struct JogCmdBinary : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> lhs, rhs;

  JogCmdBinary( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmd(t), lhs(lhs), rhs(rhs)
  { 
  }

  JogTypeInfo* type() { return lhs->type(); }

  void        validate();

  void on_push( JogVM* vm );

};

struct JogCmdLogicalOp : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdLogicalOp( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBinary(t,lhs,rhs) { }

  JogTypeInfo* type() { return jog_type_manager.type_boolean; }
};


struct JogCmdLogicalOr : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdLogicalOr( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) : JogCmdLogicalOp(t,lhs,rhs)
  { 
  }

  void print()
  {
    lhs->print();
    printf("||");
    rhs->print();
  }

  Ref<JogCmd> resolve();

  void execute( JogVM* vm );
};

struct JogCmdLogicalAnd : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdLogicalAnd( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) : JogCmdLogicalOp(t,lhs,rhs)
  { 
    /*
    lhs->require_boolean();
    rhs->require_boolean();
    */
  }

  void print()
  {
    lhs->print();
    printf("&&");
    rhs->print();
  }

  Ref<JogCmd> resolve();

  void execute( JogVM* vm );
};

struct JogCmdBitwiseOr : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdBitwiseOr( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("|");
    rhs->print();
  }

  Ref<JogCmd> resolve();

  void execute( JogVM* vm );
};

struct JogCmdBitwiseXor : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdBitwiseXor( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("^");
    rhs->print();
  }

  Ref<JogCmd> resolve();

  void execute( JogVM* vm );
};

struct JogCmdBitwiseAnd : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdBitwiseAnd( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("&");
    rhs->print();
  }

  Ref<JogCmd> resolve();

  void execute( JogVM* vm );
};

struct JogCmdEQ : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdEQ( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLogicalOp(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("==");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdNE : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdNE( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLogicalOp(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("!=");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdLT : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdLT( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLogicalOp(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("<");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdLE : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdLE( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLogicalOp(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("<=");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdGT : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdGT( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLogicalOp(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf(">");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdGE : JogCmdLogicalOp
{
  int node_type() { return __LINE__; }

  JogCmdGE( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLogicalOp(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf(">=");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdInstanceOf : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>  operand;
  JogTypeInfo* of_type;

  JogCmdInstanceOf( Ref<JogToken> t, Ref<JogCmd> operand, JogTypeInfo* of_type )
    : JogCmd(t), operand(operand), of_type(of_type)
  { 
    //operand->require_reference();
  }

  JogTypeInfo* type() { return jog_type_manager.type_boolean; }

  void print()
  {
    operand->print();
    printf(" instanceof ");
    of_type->print();
  }
};

struct JogCmdShift : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> operand;
  Ref<JogCmd> shift_amount;

  JogCmdShift( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmd(t), operand(operand), shift_amount(shift_amount)
  {
    //operand->require_integer();
    //this->shift_amount = shift_amount->cast_to_type( jog_type_manager.type_int32 );
  }

  JogTypeInfo* type() { return operand->type(); }

  void on_push( JogVM* vm );
};

struct JogCmdLeftShift : JogCmdShift
{
  int node_type() { return __LINE__; }

  JogCmdLeftShift( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve();

  void print()
  {
    operand->print();
    printf("<<");
    shift_amount->print();
  }
};

struct JogCmdRightShift : JogCmdShift
{
  int node_type() { return __LINE__; }

  JogCmdRightShift( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve();

  void print()
  {
    operand->print();
    printf(">>>");
    shift_amount->print();
  }
};

struct JogCmdRightXShift : JogCmdShift
{
  int node_type() { return __LINE__; }

  JogCmdRightXShift( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve();

  void print()
  {
    operand->print();
    printf(">>");
    shift_amount->print();
  }
};

struct JogCmdAdd : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdAdd( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("+");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdSub : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdSub( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("-");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdMul : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdMul( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("*");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};


struct JogCmdDiv : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdDiv( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("/");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdMod : JogCmdBinary
{
  int node_type() { return __LINE__; }

  JogCmdMod( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) : JogCmdBinary(t,lhs,rhs) { }

  void print()
  {
    lhs->print();
    printf("%%");
    rhs->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdDataType : JogCmd
{
  int node_type() { return __LINE__; }

  JogTypeInfo* data_type;

  JogCmdDataType( Ref<JogToken> t, JogTypeInfo* data_type ) : JogCmd(t), data_type(data_type)
  {
  }

  JogTypeInfo* type() { return data_type; }

  void print()
  {
    data_type->print();
  }
};

struct JogCmdIdentifier : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogString> name;

  JogCmdIdentifier( Ref<JogToken> t, Ref<JogString> name ) : JogCmd(t), name(name)
  {
  }

  JogTypeInfo* type() { return NULL; }

  JogTypeInfo* reinterpret_as_type()
  {
    return (JogTypeInfo::reference( t, name ));
  }

  void print()
  {
    name->print();
  }

  Ref<JogCmd> resolve();

  Ref<JogCmd> resolve( Ref<JogCmd> context );
  Ref<JogCmd> resolve( JogTypeInfo* class_context, Ref<JogCmd> context );

  Ref<JogCmd> resolve_assignment( Ref<JogCmd> context, Ref<JogCmd> new_value );
  Ref<JogCmd> resolve_assignment( JogTypeInfo* class_context, Ref<JogCmd> context, 
                                  Ref<JogCmd> new_value );

  Ref<JogCmd> resolve_op_assign( int op_type, Ref<JogCmd> context, Ref<JogCmd> rhs );

  Ref<JogCmd> resolve_stepcount_access( int when, int modifier );
  Ref<JogCmd> resolve_stepcount_access( int when, int modifier,
                JogTypeInfo* class_context, Ref<JogCmd> context );
  Ref<JogCmd> resolve_stepcount_access( int when, int modifier, Ref<JogCmd> context );
};

struct JogCmdLocalVarDeclaration : JogCmdIdentifier
{
  int node_type() { return __LINE__; }

  JogTypeInfo* of_type;
  Ref<JogCmd>  initial_value;
  JogLocalVarInfo* var_info;

  JogCmdLocalVarDeclaration( Ref<JogToken> t, JogTypeInfo* of_type, Ref<JogString> name )
    : JogCmdIdentifier(t,name), of_type(of_type), initial_value(NULL), var_info(NULL)
  {
  }

  JogTypeInfo* reinterpret_as_type() { return NULL; }

  void print()
  {
    of_type->print();
    printf(" ");
    JogCmdIdentifier::print();
    if (*initial_value)
    {
      printf("=");
      initial_value->print();
    }
  }

  Ref<JogCmd> resolve();
};

struct JogCmdMethodCall : JogCmdIdentifier
{
  int node_type() { return __LINE__; }

  Ref<JogCmdList> args;

  JogCmdMethodCall( Ref<JogToken> t, Ref<JogString> name, Ref<JogCmdList> args ) 
    : JogCmdIdentifier(t,name), args(args)
  {
  }

  JogTypeInfo* reinterpret_as_type() { return NULL; }

  void print()
  {
    JogCmdIdentifier::print();
    args->print();
  }

  Ref<JogCmd> resolve();
  Ref<JogCmd> resolve( Ref<JogCmd> context );
  Ref<JogCmd> resolve( JogTypeInfo* class_context, Ref<JogCmd> context );
};

struct JogCmdSuperCall : JogCmdMethodCall
{
  int node_type() { return __LINE__; }

  JogCmdSuperCall( Ref<JogToken> t, Ref<JogString> name, Ref<JogCmdList> args ) 
    : JogCmdMethodCall(t,name,args)
  {
  }

  void print()
  {
    printf("super.");
    JogCmdIdentifier::print();
    args->print();
  }

  Ref<JogCmd> resolve();
  Ref<JogCmd> resolve( Ref<JogCmd> context );
  Ref<JogCmd> resolve( JogTypeInfo* class_context, Ref<JogCmd> context );
};

struct JogCmdNewObject : JogCmd
{
  int node_type() { return __LINE__; }

  JogTypeInfo*    of_type;
  Ref<JogCmdList> args;
  JogMethodInfo*  method_info;

  JogCmdNewObject( Ref<JogToken> t, JogTypeInfo* of_type, Ref<JogCmdList> args ) 
    : JogCmd(t), of_type(of_type), args(args), method_info(NULL)
  {
  }

  JogTypeInfo* type() { return of_type; }

  void print()
  {
    printf("new ");
    of_type->print();
    args->print();
  }

  Ref<JogCmd> resolve();

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};


struct JogCmdStepCount : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> operand;

  JogCmdStepCount( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmd(t), operand(operand)
  {
  }

  JogTypeInfo* type() { return operand->type(); }
};

struct JogCmdPreIncrement : JogCmdStepCount
{
  int node_type() { return __LINE__; }

  JogCmdPreIncrement( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdStepCount(t,operand)
  {
  }

  void print()
  {
    printf("++");
    operand->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdPreDecrement : JogCmdStepCount
{
  int node_type() { return __LINE__; }

  JogCmdPreDecrement( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdStepCount(t,operand)
  {
  }

  void print()
  {
    printf("--");
    operand->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdPostIncrement : JogCmdStepCount
{
  int node_type() { return __LINE__; }

  JogCmdPostIncrement( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdStepCount(t,operand)
  {
  }

  void print()
  {
    operand->print();
    printf("++");
  }

  Ref<JogCmd> resolve();
};

struct JogCmdPostDecrement : JogCmdStepCount
{
  int node_type() { return __LINE__; }

  JogCmdPostDecrement( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdStepCount(t,operand)
  {
  }

  void print()
  {
    operand->print();
    printf("--");
  }

  Ref<JogCmd> resolve();
};

struct JogCmdNegate : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdNegate( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  {
  }

  Ref<JogCmd> resolve();

  void print()
  {
    printf("-");
    operand->print();
  }
};

struct JogCmdLogicalNot : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdLogicalNot( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  {
  }

  void print()
  {
    printf("!");
    operand->print();
  }

  Ref<JogCmd> resolve();

  void execute( JogVM* vm ) ;
};

struct JogCmdBitwiseNot : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdBitwiseNot( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  {
  }

  void print()
  {
    printf("~");
    operand->print();
  }

  Ref<JogCmd> resolve();

  void execute( JogVM* vm ) ;
};

struct JogCmdMemberAccess : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> context;
  Ref<JogCmd> member;

  JogCmdMemberAccess( Ref<JogToken> t, Ref<JogCmd> context, Ref<JogCmd> member )
    : JogCmd(t), context(context), member(member)
  {
  }

  void print()
  {
    context->print();
    printf(".");
    member->print();
  }

  JogTypeInfo* type() { return NULL; }

  Ref<JogCmd> resolve();
  Ref<JogCmd> resolve_assignment( Ref<JogCmd> assignment_context, Ref<JogCmd> new_value );
  Ref<JogCmd> resolve_stepcount_access( int when, int modifier );
};

struct JogCmdArrayAccess : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd> context;
  Ref<JogCmd> index;

  JogCmdArrayAccess( Ref<JogToken> t, Ref<JogCmd> context, Ref<JogCmd> index )
    : JogCmd(t), context(context), index(index)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    printf("(");
    context->print();
    printf("[");
    index->print();
    printf("]");
    printf(")");
  }
};

struct JogCmdReturnValue : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdReturnValue( Ref<JogToken> t, Ref<JogCmd> operand ) : JogCmdUnary(t,operand)
  {
  }

  void print()
  {
    printf("return ");
    operand->print();
  }

  Ref<JogCmd> resolve();
};

struct JogCmdCallSuperConstructor : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmdList> args;

  JogCmdCallSuperConstructor( Ref<JogToken> t, Ref<JogCmdList> args ) : JogCmd(t), args(args)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    printf("super");
    args->print();
  }

  Ref<JogCmd> resolve();
};


//=============================================================================
//  VM Commands
//=============================================================================
struct JogCmdCastReal32ToReal64 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastReal32ToReal64( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_real32; }

  void print()
  {
    printf( "(double)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};


struct JogCmdCastIntegerToReal64 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastIntegerToReal64( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_real64; }

  void print()
  {
    printf( "(double)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};


struct JogCmdCastReal64ToReal32 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastReal64ToReal32( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_real32; }

  void print()
  {
    printf( "(float)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCastIntegerToReal32 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastIntegerToReal32( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_real32; }

  void print()
  {
    printf( "(float)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCastRealToInt64 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastRealToInt64( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_int64; }

  void print()
  {
    printf( "(long)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCastIntegerToInt64 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastIntegerToInt64( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_int64; }

  void print()
  {
    printf( "(long)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};


struct JogCmdCastRealToInt32 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastRealToInt32( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_int32; }

  void print()
  {
    printf( "(int)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCastIntegerToInt32 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastIntegerToInt32( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_int32; }

  void print()
  {
    printf( "(int)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCastIntegerToInt16 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastIntegerToInt16( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_int16; }

  void print()
  {
    printf( "(short)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCastIntegerToInt8 : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastIntegerToInt8( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_int8; }

  void print()
  {
    printf( "(byte)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCastIntegerToChar : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdCastIntegerToChar( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdUnary(t,operand)
  { 
  }

  JogTypeInfo* type() { return jog_type_manager.type_char; }

  void print()
  {
    printf( "(char)" );
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

//------------------------------------------------------------------------------
//  Add
//------------------------------------------------------------------------------
struct JogCmdAddReal64 : JogCmdAdd
{
  int node_type() { return __LINE__; }

  JogCmdAddReal64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdAdd(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdAddReal32 : JogCmdAdd
{
  int node_type() { return __LINE__; }

  JogCmdAddReal32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdAdd(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdAddInt64 : JogCmdAdd
{
  int node_type() { return __LINE__; }

  JogCmdAddInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdAdd(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdAddInt32 : JogCmdAdd
{
  int node_type() { return __LINE__; }

  JogCmdAddInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdAdd(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  Sub
//------------------------------------------------------------------------------
struct JogCmdSubReal64 : JogCmdSub
{
  int node_type() { return __LINE__; }

  JogCmdSubReal64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdSub(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdSubReal32 : JogCmdSub
{
  int node_type() { return __LINE__; }

  JogCmdSubReal32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdSub(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdSubInt64 : JogCmdSub
{
  int node_type() { return __LINE__; }

  JogCmdSubInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdSub(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdSubInt32 : JogCmdSub
{
  int node_type() { return __LINE__; }

  JogCmdSubInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdSub(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  Mul
//------------------------------------------------------------------------------
struct JogCmdMulReal64 : JogCmdMul
{
  int node_type() { return __LINE__; }

  JogCmdMulReal64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdMul(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdMulReal32 : JogCmdMul
{
  int node_type() { return __LINE__; }

  JogCmdMulReal32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdMul(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdMulInt64 : JogCmdMul
{
  int node_type() { return __LINE__; }

  JogCmdMulInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdMul(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdMulInt32 : JogCmdMul
{
  int node_type() { return __LINE__; }

  JogCmdMulInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdMul(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  Div
//------------------------------------------------------------------------------
struct JogCmdDivReal64 : JogCmdDiv
{
  int node_type() { return __LINE__; }

  JogCmdDivReal64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdDiv(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdDivReal32 : JogCmdDiv
{
  int node_type() { return __LINE__; }

  JogCmdDivReal32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdDiv(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdDivInt64 : JogCmdDiv
{
  int node_type() { return __LINE__; }

  JogCmdDivInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdDiv(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdDivInt32 : JogCmdDiv
{
  int node_type() { return __LINE__; }

  JogCmdDivInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdDiv(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  Mod
//------------------------------------------------------------------------------
struct JogCmdModInt64 : JogCmdMod
{
  int node_type() { return __LINE__; }

  JogCmdModInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdMod(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdModInt32 : JogCmdMod
{
  int node_type() { return __LINE__; }

  JogCmdModInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdMod(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

//------------------------------------------------------------------------------
//  And
//------------------------------------------------------------------------------
struct JogCmdAndInt64 : JogCmdBitwiseAnd
{
  int node_type() { return __LINE__; }

  JogCmdAndInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBitwiseAnd(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdAndInt32 : JogCmdBitwiseAnd
{
  int node_type() { return __LINE__; }

  JogCmdAndInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBitwiseAnd(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  Or
//------------------------------------------------------------------------------
struct JogCmdOrInt64 : JogCmdBitwiseOr
{
  int node_type() { return __LINE__; }

  JogCmdOrInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBitwiseOr(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdOrInt32 : JogCmdBitwiseOr
{
  int node_type() { return __LINE__; }

  JogCmdOrInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBitwiseOr(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  Xor
//------------------------------------------------------------------------------
struct JogCmdXorInt64 : JogCmdBitwiseXor
{
  int node_type() { return __LINE__; }

  JogCmdXorInt64( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBitwiseXor(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdXorInt32 : JogCmdBitwiseXor
{
  int node_type() { return __LINE__; }

  JogCmdXorInt32( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdBitwiseXor(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};



//------------------------------------------------------------------------------
//  EQ
//------------------------------------------------------------------------------
struct JogCmdEQReal : JogCmdEQ
{
  int node_type() { return __LINE__; }

  JogCmdEQReal( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdEQ(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdEQInteger : JogCmdEQ
{
  int node_type() { return __LINE__; }

  JogCmdEQInteger( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdEQ(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdEQRef : JogCmdEQ
{
  int node_type() { return __LINE__; }

  JogCmdEQRef( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdEQ(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  NE
//------------------------------------------------------------------------------
struct JogCmdNEReal : JogCmdNE
{
  int node_type() { return __LINE__; }

  JogCmdNEReal( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdNE(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNEInteger : JogCmdNE
{
  int node_type() { return __LINE__; }

  JogCmdNEInteger( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdNE(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNERef : JogCmdNE
{
  int node_type() { return __LINE__; }

  JogCmdNERef( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdNE(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  LT
//------------------------------------------------------------------------------
struct JogCmdLTReal : JogCmdLT
{
  int node_type() { return __LINE__; }

  JogCmdLTReal( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLT(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdLTInteger : JogCmdLT
{
  int node_type() { return __LINE__; }

  JogCmdLTInteger( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLT(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  LE
//------------------------------------------------------------------------------
struct JogCmdLEReal : JogCmdLE
{
  int node_type() { return __LINE__; }

  JogCmdLEReal( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLE(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdLEInteger : JogCmdLE
{
  int node_type() { return __LINE__; }

  JogCmdLEInteger( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdLE(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  GT
//------------------------------------------------------------------------------
struct JogCmdGTReal : JogCmdGT
{
  int node_type() { return __LINE__; }

  JogCmdGTReal( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdGT(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdGTInteger : JogCmdGT
{
  int node_type() { return __LINE__; }

  JogCmdGTInteger( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdGT(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//------------------------------------------------------------------------------
//  GE
//------------------------------------------------------------------------------
struct JogCmdGEReal : JogCmdGE
{
  int node_type() { return __LINE__; }

  JogCmdGEReal( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdGE(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdGEInteger : JogCmdGE
{
  int node_type() { return __LINE__; }

  JogCmdGEInteger( Ref<JogToken> t, Ref<JogCmd> lhs, Ref<JogCmd> rhs ) 
    : JogCmdGE(t,lhs,rhs) 
  { 
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//-----------------------------------------------------------------------------
//  LeftShift
//-----------------------------------------------------------------------------
struct JogCmdLeftShiftInt64 : JogCmdLeftShift
{
  int node_type() { return __LINE__; }

  JogCmdLeftShiftInt64( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdLeftShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdLeftShiftInt32 : JogCmdLeftShift
{
  int node_type() { return __LINE__; }

  JogCmdLeftShiftInt32( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdLeftShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//-----------------------------------------------------------------------------
//  RightShift
//-----------------------------------------------------------------------------
struct JogCmdRightShiftInt64 : JogCmdRightShift
{
  int node_type() { return __LINE__; }

  JogCmdRightShiftInt64( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdRightShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdRightShiftInt32 : JogCmdRightShift
{
  int node_type() { return __LINE__; }

  JogCmdRightShiftInt32( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdRightShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//-----------------------------------------------------------------------------
//  RightXShift
//-----------------------------------------------------------------------------
struct JogCmdRightXShiftInt64 : JogCmdRightXShift
{
  int node_type() { return __LINE__; }

  JogCmdRightXShiftInt64( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdRightXShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdRightXShiftInt32 : JogCmdRightXShift
{
  int node_type() { return __LINE__; }

  JogCmdRightXShiftInt32( Ref<JogToken> t, Ref<JogCmd> operand, Ref<JogCmd> shift_amount ) 
    : JogCmdRightXShift( t, operand, shift_amount )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


//-----------------------------------------------------------------------------
//  Negate
//-----------------------------------------------------------------------------
struct JogCmdNegateReal64 : JogCmdNegate
{
  int node_type() { return __LINE__; }

  JogCmdNegateReal64( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdNegate( t, operand )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNegateReal32 : JogCmdNegate
{
  int node_type() { return __LINE__; }

  JogCmdNegateReal32( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdNegate( t, operand )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNegateInt64 : JogCmdNegate
{
  int node_type() { return __LINE__; }

  JogCmdNegateInt64( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdNegate( t, operand )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNegateInt32 : JogCmdNegate
{
  int node_type() { return __LINE__; }

  JogCmdNegateInt32( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdNegate( t, operand )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNegateInt16 : JogCmdNegate
{
  int node_type() { return __LINE__; }

  JogCmdNegateInt16( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdNegate( t, operand )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNegateInt8 : JogCmdNegate
{
  int node_type() { return __LINE__; }

  JogCmdNegateInt8( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdNegate( t, operand )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};

struct JogCmdNegateChar : JogCmdNegate
{
  int node_type() { return __LINE__; }

  JogCmdNegateChar( Ref<JogToken> t, Ref<JogCmd> operand )
    : JogCmdNegate( t, operand )
  {
  }

  Ref<JogCmd> resolve() { return this; }

  void execute( JogVM* vm );
};


/*
while (x++ < 8)
{
  if (x == 7) break;
}
*/

struct JogCmdThis : JogCmd
{
  int node_type() { return __LINE__; }

  JogTypeInfo* this_type;

  JogCmdThis( Ref<JogToken> t, JogTypeInfo* this_type );

  JogTypeInfo* type() { return this_type; }

  void print()
  {
    printf("this");
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdNullRef : JogCmd
{
  int node_type() { return __LINE__; }

  JogTypeInfo* of_type;

  JogCmdNullRef( Ref<JogToken> t ) : JogCmd(t)
  {
    of_type = jog_type_manager.type_null;
  }

  JogTypeInfo* type() { return of_type; }

  void print()
  {
    if (of_type == jog_type_manager.type_null) printf("null");
    else
    {
      printf("(");
      of_type->name->print();
      printf(")null");
    }
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdObjectRef : JogCmd
{
  int node_type() { return __LINE__; }

  JogRef ref;

  JogCmdObjectRef( Ref<JogToken> t, JogRef ref ) : JogCmd(t), ref(ref)
  {
  }

  JogTypeInfo* type() { return (*ref) ? ref->type : NULL; }

  void print()
  {
    printf("(objref)");
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdCallInitObject : JogCmd
{
  int node_type() { return __LINE__; }

  JogMethodInfo*  method_info;

  JogCmdCallInitObject( Ref<JogToken> t, JogMethodInfo* method_info )
    : JogCmd(t), method_info(method_info)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    method_info->name->print();
    printf("()");
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};



struct JogCmdStaticCall : JogCmd
{
  int node_type() { return __LINE__; }

  JogMethodInfo*  method_info;
  Ref<JogCmd>     context;
  Ref<JogCmdList> args;

  JogCmdStaticCall( Ref<JogToken> t, JogMethodInfo* method_info, 
      Ref<JogCmd> context, Ref<JogCmdList> args )
    : JogCmd(t), method_info(method_info), context(context), args(args)
  {
  }

  JogTypeInfo* type() { return method_info->return_type; }

  void print()
  {
    if (*context)
    {
      context->print();
      printf(".");
    }
    method_info->name->print();
    if (*args) args->print();
    else printf("()");
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};


struct JogCmdDynamicCall : JogCmdStaticCall
{
  int node_type() { return __LINE__; }

  JogCmdDynamicCall( Ref<JogToken> t, JogMethodInfo* method_info, 
      Ref<JogCmd> context, Ref<JogCmdList> args )
    : JogCmdStaticCall( t, method_info, context, args )
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdClassCall : JogCmd
{
  int node_type() { return __LINE__; }

  JogMethodInfo*  method_info;
  Ref<JogCmd>     context;
  Ref<JogCmdList> args;

  JogCmdClassCall( Ref<JogToken> t, JogMethodInfo* method_info, 
      Ref<JogCmd> context, Ref<JogCmdList> args )
    : JogCmd(t), method_info(method_info), context(context), args(args)
  {
    if (*context) this->context = context->discarding_result();
  }

  JogTypeInfo* type() { return method_info->return_type; }

  void print()
  {
    method_info->name->print();
    if (*args) args->print();
    else printf("()");
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdReturnData : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdReturnData( Ref<JogToken> t, Ref<JogCmd> operand ) : JogCmdUnary(t,operand)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    printf("returnData ");
    operand->print();
  }

  void on_push( JogVM* vm );

  void execute( JogVM* vm );
};

struct JogCmdDiscardDataResult : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdDiscardDataResult( Ref<JogToken> t, Ref<JogCmd> operand ) : JogCmdUnary(t,operand)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    printf("discardData:");
    operand->print();
  }

  Ref<JogCmd> resolve()
  {
    operand = operand->resolve();
    return this;
  }

  void execute( JogVM* vm );
};

struct JogCmdDiscardRefResult : JogCmdUnary
{
  int node_type() { return __LINE__; }

  JogCmdDiscardRefResult( Ref<JogToken> t, Ref<JogCmd> operand ) : JogCmdUnary(t,operand)
  {
  }

  JogTypeInfo* type() { return NULL; }

  void print()
  {
    printf("discardRef:");
    operand->print();
  }

  Ref<JogCmd> resolve()
  {
    operand = operand->resolve();
    return this;
  }

  void execute( JogVM* vm );
};

//=============================================================================
//  ReadClassProperty
//=============================================================================
struct JogCmdReadClassProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context;  // can be null
  JogPropertyInfo* var_info;

  JogCmdReadClassProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info )
    : JogCmd(t), context(context), var_info(var_info)
  {
    if (*context) this->context = context->discarding_result();
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    if (*context) context->print();
    else var_info->type_context->name->print();
    printf(".");
    var_info->name->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdReadClassPropertyData : JogCmdReadClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdReadClassPropertyData( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info )
    : JogCmdReadClassProperty(t,context,var_info)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdReadClassPropertyRef : JogCmdReadClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdReadClassPropertyRef( Ref<JogToken> t,
      Ref<JogCmd> context, JogPropertyInfo* var_info )
    : JogCmdReadClassProperty(t,context,var_info)
  {
  }

  void execute( JogVM* vm );
};


//=============================================================================
//  WriteClassProperty
//=============================================================================
struct JogCmdWriteClassProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context, new_value;
  JogPropertyInfo* var_info;

  JogCmdWriteClassProperty( Ref<JogToken> t, 
      Ref<JogCmd> context, JogPropertyInfo* var_info,
      Ref<JogCmd> new_value )
    : JogCmd(t), context(context), 
      new_value(new_value), var_info(var_info)
  {
    if (*context)
    {
      JogTypeInfo* result_type = context->type();
      if (result_type)
      {
        if (result_type->is_primitive())
        {
          this->context = new JogCmdDiscardDataResult(context->t,context);
        }
        else
        {
          this->context = new JogCmdDiscardRefResult(context->t,context);
        }
      }
    }
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    if (*context) context->print();
    else var_info->type_context->name->print();
    printf(".");
    var_info->name->print();
    printf(" = ");
    new_value->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdWriteClassPropertyData : JogCmdWriteClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdWriteClassPropertyData( Ref<JogToken> t, 
      Ref<JogCmd> context, JogPropertyInfo* var_info,
      Ref<JogCmd> new_value )
    : JogCmdWriteClassProperty(t,context,var_info,new_value)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdWriteClassPropertyRef : JogCmdWriteClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdWriteClassPropertyRef( Ref<JogToken> t,
      Ref<JogCmd> context, JogPropertyInfo* var_info,
      Ref<JogCmd> new_value )
    : JogCmdWriteClassProperty(t,context,var_info,new_value)
  {
  }

  void execute( JogVM* vm );
};



//=============================================================================
//  ReadProperty
//=============================================================================
struct JogCmdReadProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context;
  JogPropertyInfo* var_info;

  JogCmdReadProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info )
    : JogCmd(t), context(context), var_info(var_info)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdReadPropertyData : JogCmdReadProperty
{
  int node_type() { return __LINE__; }

  JogCmdReadPropertyData( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info )
    : JogCmdReadProperty(t,context,var_info)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdReadPropertyRef : JogCmdReadProperty
{
  int node_type() { return __LINE__; }

  JogCmdReadPropertyRef( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info )
    : JogCmdReadProperty(t,context,var_info)
  {
  }

  void execute( JogVM* vm );
};


//=============================================================================
//  WriteProperty
//=============================================================================
struct JogCmdWriteProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context, new_value;
  JogPropertyInfo* var_info;

  JogCmdWriteProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info,
      Ref<JogCmd> new_value )
    : JogCmd(t), context(context), new_value(new_value), var_info(var_info)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" = ");
    new_value->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdWritePropertyData : JogCmdWriteProperty
{
  int node_type() { return __LINE__; }

  JogCmdWritePropertyData( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info,
      Ref<JogCmd> new_value )
    : JogCmdWriteProperty(t,context,var_info,new_value)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdWritePropertyRef : JogCmdWriteProperty
{
  int node_type() { return __LINE__; }

  JogCmdWritePropertyRef( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info,
      Ref<JogCmd> new_value )
    : JogCmdWriteProperty(t,context,var_info,new_value)
  {
  }

  void execute( JogVM* vm );
};



//=============================================================================
//  ReadLocal
//=============================================================================
struct JogCmdReadLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;

  JogCmdReadLocal( Ref<JogToken> t, JogLocalVarInfo* var_info )
    : JogCmd(t), var_info(var_info)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    printf("(local:");
    var_info->name->print();
    printf(")");
  }

  void on_push( JogVM* vm );
};

struct JogCmdReadLocalData : JogCmdReadLocal
{
  int node_type() { return __LINE__; }

  JogCmdReadLocalData( Ref<JogToken> t, JogLocalVarInfo* var_info )
    : JogCmdReadLocal(t,var_info)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdReadLocalRef : JogCmdReadLocal
{
  int node_type() { return __LINE__; }

  JogCmdReadLocalRef( Ref<JogToken> t, JogLocalVarInfo* var_info )
    : JogCmdReadLocal(t,var_info)
  {
  }

  void execute( JogVM* vm );
};


//=============================================================================
//  WriteLocal
//=============================================================================
struct JogCmdWriteLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      new_value;

  JogCmdWriteLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> new_value ) : 
    JogCmd(t), var_info(var_info), new_value(new_value)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    printf("local:");
    var_info->name->print();
    printf(" = ");
    new_value->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdWriteLocalData : JogCmdWriteLocal
{
  int node_type() { return __LINE__; }

  JogCmdWriteLocalData( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> new_value )
    : JogCmdWriteLocal(t,var_info,new_value)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdWriteLocalRef : JogCmdWriteLocal
{
  int node_type() { return __LINE__; }

  JogCmdWriteLocalRef( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> new_value )
    : JogCmdWriteLocal(t,var_info,new_value)
  {
  }

  void execute( JogVM* vm );
};

//====================================================================
//  AddAssign Local
//====================================================================
struct JogCmdAddAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdAddAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" += ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdAddAssignLocalReal64 : JogCmdAddAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignLocalReal64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignLocalReal32 : JogCmdAddAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignLocalReal32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignLocalInt64 : JogCmdAddAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignLocalInt32 : JogCmdAddAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignLocalInt16 : JogCmdAddAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignLocalInt8 : JogCmdAddAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignLocalChar : JogCmdAddAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SubAssign Local
//====================================================================
struct JogCmdSubAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdSubAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" -= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSubAssignLocalReal64 : JogCmdSubAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignLocalReal64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignLocalReal32 : JogCmdSubAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignLocalReal32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignLocalInt64 : JogCmdSubAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignLocalInt32 : JogCmdSubAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignLocalInt16 : JogCmdSubAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignLocalInt8 : JogCmdSubAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignLocalChar : JogCmdSubAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  MulAssign Local
//====================================================================
struct JogCmdMulAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdMulAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" *= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdMulAssignLocalReal64 : JogCmdMulAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignLocalReal64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignLocalReal32 : JogCmdMulAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignLocalReal32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignLocalInt64 : JogCmdMulAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignLocalInt32 : JogCmdMulAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignLocalInt16 : JogCmdMulAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignLocalInt8 : JogCmdMulAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignLocalChar : JogCmdMulAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  DivAssign Local
//====================================================================
struct JogCmdDivAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdDivAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" /= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdDivAssignLocalReal64 : JogCmdDivAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignLocalReal64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignLocalReal32 : JogCmdDivAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignLocalReal32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignLocalInt64 : JogCmdDivAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignLocalInt32 : JogCmdDivAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignLocalInt16 : JogCmdDivAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignLocalInt8 : JogCmdDivAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignLocalChar : JogCmdDivAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  ModAssign Local
//====================================================================
struct JogCmdModAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdModAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" %%= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdModAssignLocalInt64 : JogCmdModAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdModAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignLocalInt32 : JogCmdModAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdModAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignLocalInt16 : JogCmdModAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdModAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignLocalInt8 : JogCmdModAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdModAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignLocalChar : JogCmdModAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdModAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  AndAssign Local
//====================================================================
struct JogCmdAndAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdAndAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" &= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdAndAssignLocalInt64 : JogCmdAndAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignLocalInt32 : JogCmdAndAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignLocalInt16 : JogCmdAndAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignLocalInt8 : JogCmdAndAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignLocalChar : JogCmdAndAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  OrAssign Local
//====================================================================
struct JogCmdOrAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdOrAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" |= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdOrAssignLocalInt64 : JogCmdOrAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignLocalInt32 : JogCmdOrAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignLocalInt16 : JogCmdOrAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignLocalInt8 : JogCmdOrAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignLocalChar : JogCmdOrAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  XorAssign Local
//====================================================================
struct JogCmdXorAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdXorAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" ^= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdXorAssignLocalInt64 : JogCmdXorAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignLocalInt32 : JogCmdXorAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignLocalInt16 : JogCmdXorAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignLocalInt8 : JogCmdXorAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignLocalChar : JogCmdXorAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SHLAssign Local
//====================================================================
struct JogCmdSHLAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdSHLAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" <<= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSHLAssignLocalInt64 : JogCmdSHLAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignLocalInt32 : JogCmdSHLAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignLocalInt16 : JogCmdSHLAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignLocalInt8 : JogCmdSHLAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignLocalChar : JogCmdSHLAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SHRAssign Local
//====================================================================
struct JogCmdSHRAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdSHRAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" >>>= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSHRAssignLocalInt64 : JogCmdSHRAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignLocalInt32 : JogCmdSHRAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignLocalInt16 : JogCmdSHRAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignLocalInt8 : JogCmdSHRAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignLocalChar : JogCmdSHRAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SHRXAssign Local
//====================================================================
struct JogCmdSHRXAssignLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  Ref<JogCmd>      operand;

  JogCmdSHRXAssignLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    printf(" >>= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSHRXAssignLocalInt64 : JogCmdSHRXAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignLocalInt32 : JogCmdSHRXAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignLocalInt16 : JogCmdSHRXAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignLocalInt8 : JogCmdSHRXAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignLocalChar : JogCmdSHRXAssignLocal
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignLocal(t,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  AddAssign Property
//====================================================================
struct JogCmdAddAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdAddAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" += ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdAddAssignPropertyReal64 : JogCmdAddAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignPropertyReal32 : JogCmdAddAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignPropertyInt64 : JogCmdAddAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignPropertyInt32 : JogCmdAddAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignPropertyInt16 : JogCmdAddAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignPropertyInt8 : JogCmdAddAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAddAssignPropertyChar : JogCmdAddAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAddAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAddAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SubAssign Property
//====================================================================
struct JogCmdSubAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdSubAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" -= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSubAssignPropertyReal64 : JogCmdSubAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignPropertyReal32 : JogCmdSubAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignPropertyInt64 : JogCmdSubAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignPropertyInt32 : JogCmdSubAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignPropertyInt16 : JogCmdSubAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignPropertyInt8 : JogCmdSubAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSubAssignPropertyChar : JogCmdSubAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSubAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSubAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  MulAssign Property
//====================================================================
struct JogCmdMulAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdMulAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" *= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdMulAssignPropertyReal64 : JogCmdMulAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignPropertyReal32 : JogCmdMulAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignPropertyInt64 : JogCmdMulAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignPropertyInt32 : JogCmdMulAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignPropertyInt16 : JogCmdMulAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignPropertyInt8 : JogCmdMulAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdMulAssignPropertyChar : JogCmdMulAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdMulAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdMulAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  DivAssign Property
//====================================================================
struct JogCmdDivAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdDivAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" /= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdDivAssignPropertyReal64 : JogCmdDivAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignPropertyReal32 : JogCmdDivAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignPropertyInt64 : JogCmdDivAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignPropertyInt32 : JogCmdDivAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignPropertyInt16 : JogCmdDivAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignPropertyInt8 : JogCmdDivAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdDivAssignPropertyChar : JogCmdDivAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdDivAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdDivAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  ModAssign Property
//====================================================================
struct JogCmdModAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdModAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" %%= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdModAssignPropertyInt64 : JogCmdModAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdModAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignPropertyInt32 : JogCmdModAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdModAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignPropertyInt16 : JogCmdModAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdModAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignPropertyInt8 : JogCmdModAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdModAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdModAssignPropertyChar : JogCmdModAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdModAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdModAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  AndAssign Property
//====================================================================
struct JogCmdAndAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdAndAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" &= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdAndAssignPropertyInt64 : JogCmdAndAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignPropertyInt32 : JogCmdAndAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignPropertyInt16 : JogCmdAndAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignPropertyInt8 : JogCmdAndAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdAndAssignPropertyChar : JogCmdAndAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdAndAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdAndAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  OrAssign Property
//====================================================================
struct JogCmdOrAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdOrAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" |= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdOrAssignPropertyInt64 : JogCmdOrAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignPropertyInt32 : JogCmdOrAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignPropertyInt16 : JogCmdOrAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignPropertyInt8 : JogCmdOrAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdOrAssignPropertyChar : JogCmdOrAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdOrAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdOrAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  XorAssign Property
//====================================================================
struct JogCmdXorAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdXorAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" ^= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdXorAssignPropertyInt64 : JogCmdXorAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignPropertyInt32 : JogCmdXorAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignPropertyInt16 : JogCmdXorAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignPropertyInt8 : JogCmdXorAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdXorAssignPropertyChar : JogCmdXorAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdXorAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdXorAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SHLAssign Property
//====================================================================
struct JogCmdSHLAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdSHLAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" <<= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSHLAssignPropertyInt64 : JogCmdSHLAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignPropertyInt32 : JogCmdSHLAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignPropertyInt16 : JogCmdSHLAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignPropertyInt8 : JogCmdSHLAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHLAssignPropertyChar : JogCmdSHLAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHLAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHLAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SHRAssign Property
//====================================================================
struct JogCmdSHRAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdSHRAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" >>>= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSHRAssignPropertyInt64 : JogCmdSHRAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignPropertyInt32 : JogCmdSHRAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignPropertyInt16 : JogCmdSHRAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignPropertyInt8 : JogCmdSHRAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRAssignPropertyChar : JogCmdSHRAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  SHRXAssign Property
//====================================================================
struct JogCmdSHRXAssignProperty : JogCmd
{
  int node_type() { return __LINE__; }

  JogPropertyInfo* var_info;
  Ref<JogCmd>      operand, context;

  JogCmdSHRXAssignProperty( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand ) : 
    JogCmd(t), var_info(var_info), operand(operand), context(context)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    printf(" >>= ");
    operand->print();
  }

  void on_push( JogVM* vm );
};

struct JogCmdSHRXAssignPropertyInt64 : JogCmdSHRXAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignPropertyInt32 : JogCmdSHRXAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignPropertyInt16 : JogCmdSHRXAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignPropertyInt8 : JogCmdSHRXAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdSHRXAssignPropertyChar : JogCmdSHRXAssignProperty
{
  int node_type() { return __LINE__; }

  JogCmdSHRXAssignPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, JogPropertyInfo* var_info, Ref<JogCmd> operand )
    : JogCmdSHRXAssignProperty(t,context,var_info,operand)
  {
  }

  void execute( JogVM* vm );
};


//====================================================================
//  StepAccess
//====================================================================
struct JogCmdPreStepLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  int              modifier;

  JogCmdPreStepLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmd(t), var_info(var_info), modifier(modifier)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    if (modifier > 0) printf("++");
    else              printf("--");
    var_info->name->print();
  }

  Ref<JogCmd> resolve() { return this; }

  void on_push( JogVM* vm ) { }
};

struct JogCmdPreStepLocalReal64 : JogCmdPreStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPreStepLocalReal64( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPreStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepLocalReal32 : JogCmdPreStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPreStepLocalReal32( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPreStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepLocalInt64 : JogCmdPreStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPreStepLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPreStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepLocalInt32 : JogCmdPreStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPreStepLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPreStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepLocalInt16 : JogCmdPreStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPreStepLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPreStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepLocalInt8 : JogCmdPreStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPreStepLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPreStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepLocalChar : JogCmdPreStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPreStepLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPreStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};


struct JogCmdPostStepLocal : JogCmd
{
  int node_type() { return __LINE__; }

  JogLocalVarInfo* var_info;
  int              modifier;

  JogCmdPostStepLocal( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmd(t), var_info(var_info), modifier(modifier)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    var_info->name->print();
    if (modifier > 0) printf("++");
    else              printf("--");
  }

  Ref<JogCmd> resolve() { return this; }

  void on_push( JogVM* vm ) { }
};

struct JogCmdPostStepLocalReal64 : JogCmdPostStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPostStepLocalReal64( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPostStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepLocalReal32 : JogCmdPostStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPostStepLocalReal32( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPostStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepLocalInt64 : JogCmdPostStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPostStepLocalInt64( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPostStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepLocalInt32 : JogCmdPostStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPostStepLocalInt32( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPostStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepLocalInt16 : JogCmdPostStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPostStepLocalInt16( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPostStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepLocalInt8 : JogCmdPostStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPostStepLocalInt8( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPostStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepLocalChar : JogCmdPostStepLocal
{
  int node_type() { return __LINE__; }

  JogCmdPostStepLocalChar( Ref<JogToken> t, JogLocalVarInfo* var_info, int modifier )
    : JogCmdPostStepLocal(t,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};


//--------------------------------------------------------------------

struct JogCmdPreStepClassProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context;
  JogPropertyInfo* var_info;
  int              modifier;

  JogCmdPreStepClassProperty( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmd(t), context(context), var_info(var_info), modifier(modifier)
  {
    if (*context) this->context = context->discarding_result();
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    if (*context)
    {
      context->print();
      printf(".");
    }
    if (modifier > 0) printf("++");
    else              printf("--");
    var_info->name->print();
  }

  Ref<JogCmd> resolve() { return this; }

  void on_push( JogVM* vm );
};

struct JogCmdPreStepClassPropertyReal64 : JogCmdPreStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepClassPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepClassPropertyReal32 : JogCmdPreStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepClassPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepClassPropertyInt64 : JogCmdPreStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepClassPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepClassPropertyInt32 : JogCmdPreStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepClassPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepClassPropertyInt16 : JogCmdPreStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepClassPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepClassPropertyInt8 : JogCmdPreStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepClassPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepClassPropertyChar : JogCmdPreStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepClassPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};



struct JogCmdPostStepClassProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context;
  JogPropertyInfo* var_info;
  int              modifier;

  JogCmdPostStepClassProperty( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmd(t), context(context), var_info(var_info), modifier(modifier)
  {
    if (*context) this->context = context->discarding_result();
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    if (*context)
    {
      context->print();
      printf(".");
    }
    var_info->name->print();
    if (modifier > 0) printf("++");
    else              printf("--");
  }

  Ref<JogCmd> resolve() { return this; }

  void on_push( JogVM* vm );
};

struct JogCmdPostStepClassPropertyReal64 : JogCmdPostStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepClassPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepClassPropertyReal32 : JogCmdPostStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepClassPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepClassPropertyInt64 : JogCmdPostStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepClassPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepClassPropertyInt32 : JogCmdPostStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepClassPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepClassPropertyInt16 : JogCmdPostStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepClassPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepClassPropertyInt8 : JogCmdPostStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepClassPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepClassPropertyChar : JogCmdPostStepClassProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepClassPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepClassProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};


//--------------------------------------------------------------------

struct JogCmdPreStepProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context;
  JogPropertyInfo* var_info;
  int              modifier;

  JogCmdPreStepProperty( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmd(t), context(context), var_info(var_info), modifier(modifier)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    if (modifier > 0) printf("++");
    else              printf("--");
    var_info->name->print();
  }

  Ref<JogCmd> resolve() { return this; }

  void on_push( JogVM* vm );
};

struct JogCmdPreStepPropertyReal64 : JogCmdPreStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepPropertyReal32 : JogCmdPreStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepPropertyInt64 : JogCmdPreStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepPropertyInt32 : JogCmdPreStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepPropertyInt16 : JogCmdPreStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepPropertyInt8 : JogCmdPreStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPreStepPropertyChar : JogCmdPreStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPreStepPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPreStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};



struct JogCmdPostStepProperty : JogCmd
{
  int node_type() { return __LINE__; }

  Ref<JogCmd>      context;
  JogPropertyInfo* var_info;
  int              modifier;

  JogCmdPostStepProperty( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmd(t), context(context), var_info(var_info), modifier(modifier)
  {
  }

  JogTypeInfo* type() { return var_info->type; }

  void print()
  {
    context->print();
    printf(".");
    var_info->name->print();
    if (modifier > 0) printf("++");
    else              printf("--");
  }

  Ref<JogCmd> resolve() { return this; }

  void on_push( JogVM* vm );
};

struct JogCmdPostStepPropertyReal64 : JogCmdPostStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepPropertyReal64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepPropertyReal32 : JogCmdPostStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepPropertyReal32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepPropertyInt64 : JogCmdPostStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepPropertyInt64( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepPropertyInt32 : JogCmdPostStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepPropertyInt32( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepPropertyInt16 : JogCmdPostStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepPropertyInt16( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepPropertyInt8 : JogCmdPostStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepPropertyInt8( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};

struct JogCmdPostStepPropertyChar : JogCmdPostStepProperty
{
  int node_type() { return __LINE__; }

  JogCmdPostStepPropertyChar( Ref<JogToken> t, Ref<JogCmd> context, 
      JogPropertyInfo* var_info, int modifier )
    : JogCmdPostStepProperty(t,context,var_info,modifier)
  {
  }

  void execute( JogVM* vm );
};



//=============================================================================
//  JogParser
//=============================================================================
struct JogParser : RefCounted
{
  Ref<JogScanner> scanner;
  JogMethodInfo*  this_method;

  JogParser( const char* filename );
  JogTypeInfo* parse_type_def();
  JogTypeInfo* parse_type_def( Ref<JogToken> t, int quals, const char* missing_name_mesg );
  int parse_type_qualifiers();
  int parse_member_qualifiers();
  bool parse_member( JogTypeInfo* type );
  void parse_params( Ref<JogMethodInfo> m );
  JogTypeInfo* parse_data_type( bool parse_brackets=true );
  Ref<JogCmd> parse_initial_value();
  Ref<JogCmd> parse_statement( bool require_semicolon=true );
  Ref<JogCmd> parse_expression();
  Ref<JogCmd> parse_assignment();
  Ref<JogCmd> parse_conditional();
  Ref<JogCmd> parse_logical_or();
  Ref<JogCmd> parse_logical_or( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_logical_and();
  Ref<JogCmd> parse_logical_and( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_bitwise_or();
  Ref<JogCmd> parse_bitwise_or( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_bitwise_xor();
  Ref<JogCmd> parse_bitwise_xor( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_bitwise_and();
  Ref<JogCmd> parse_bitwise_and( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_equality();
  Ref<JogCmd> parse_equality( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_relational();
  Ref<JogCmd> parse_relational( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_shift();
  Ref<JogCmd> parse_shift( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_translate();
  Ref<JogCmd> parse_translate( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_scale();
  Ref<JogCmd> parse_scale( Ref<JogCmd> lhs );
  Ref<JogCmd> parse_prefix_unary();
  Ref<JogCmd> parse_array_decl( JogTypeInfo* array_type );
  Ref<JogCmd> parse_postfix_unary();
  Ref<JogCmd> parse_postfix_unary( Ref<JogCmd> operand );
  Ref<JogCmd> parse_term();
  Ref<JogCmd> parse_construct();
  Ref<JogCmdList> parse_args( bool required );
};

//=============================================================================
//  JogContext
//=============================================================================
struct JogContext;
extern JogContext* jog_context;

struct JogContext
{
  JogContext*    previous_context;
  JogTypeInfo*   this_type;
  JogMethodInfo* this_method;
  ArrayList<JogLocalVarInfo*> locals;

  JogContext( JogMethodInfo* m ) : this_method(m)
  {
    previous_context = jog_context;
    this_type = this_method->type_context;
    jog_context = this;

    for (int i=0; i<m->parameters.count; ++i)
    {
      add( m->parameters[i] );
    }
  }

  JogLocalVarInfo* find_local( Ref<JogString> name )
  {
    int count = locals.count;
    for (int i=0; i<count; ++i)
    {
      if (locals[i]->name->equals(name)) return locals[i];
    }
    return NULL;
  }

  void add( Ref<JogLocalVarInfo> info )
  {
    if (find_local(info->name))
    {
      StringBuilder buffer;
      buffer.print( "A local variable named '" );
      info->name->print( buffer );
      buffer.print( "' already exists in the current scope." );
      throw info->t->error( (const char*) buffer.to_string() );
    }

    info->index = this_method->locals.count;
    locals.add( *info );
    this_method->locals.add( info );
  }

  ~JogContext()
  {
    jog_context = previous_context;
  }
};

//=============================================================================
//  Native Methods
//=============================================================================
void PrintWriter__print__double( JogVM* vm );
void PrintWriter__print__int( JogVM* vm );
void PrintWriter__print__boolean( JogVM* vm );
void PrintWriter__print__char( JogVM* vm );

#endif  //JOG_H

