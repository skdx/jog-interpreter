#include "jog.h"

//=============================================================================
//  JogObject
//=============================================================================
JogObject::~JogObject()
{
  if (type->is_array()) printf("DELETE ARRAY\n");
  else printf("DELETE OBJECT\n");
}

void JogObject::release_refs()
{
  if (type->is_array())
  {
    if (type->element_type->is_reference())
    {
      JogObject** cur = ((JogObject**) data) - 1;
      int c = count + 1;
      while (--c)
      {
        JogObject* obj = *(++cur);
        if (obj) obj->release();
      }
    }
  }
  else
  {
    Ref<JogPropertyInfo>* cur = type->properties.data - 1;
    int count = type->properties.count + 1;
    while (--count)
    {
      JogPropertyInfo* p = **(++cur);
      if (p->type->is_reference())
      {
        JogObject* obj = (JogObject*)(int)data[p->index];
        if (obj) obj->release();
      }
    }
  }
}

int JogObject::total_object_bytes()
{
  if (type->is_array())
  {
    return (sizeof(JogObject) - 8) + count * type->element_type->element_size;
  }
  else
  {
    return type->object_size;
  }
}


//=============================================================================
//  JogError
//=============================================================================
void JogError::print()
{
  printf( "===============================================================================\n" );
  printf( "ERROR:   %s\n", message->data );

  if ( !(*reader) )
  {
    printf( "===============================================================================\n" );
    return;
  }

  printf( "\nAT:      \"%s\" line %d\n\n", ((JogReader*)*reader)->filename->data, line );
  printf( "CONTEXT: " );

  //printf( "CONTEXT: " );
  Ref<JogReader> r = (JogReader*) *reader;
  Ref<JogReader> context_reader = new JogReader(r);

  while (context_reader->line < line) context_reader->read();

  int c = column;
  while (c > 0 && context_reader->peek() == ' ')
  {
    --c;
    context_reader->read();
  }
  while (c > 60)
  {
    --c;
    context_reader->read();
  }
  int len = 70;
  while (len--)
  {
    int ch = context_reader->read();
    if (ch == -1 || ch == 10) break;
    printf( "%c", (char) ch );
  }
  printf("\n         ");
  while (--c > 0) printf(" ");
  printf("^\n");

  printf( "===============================================================================\n" );
}

Ref<ASCIIString> JogError::context()
{
  Ref<JogReader> context_reader = new JogReader( (JogReader*) *reader );

  StringBuilder buffer;
  while (context_reader->line < line) context_reader->read();
  while (context_reader->line == line)
  {
    int ch = context_reader->read();
    if (ch == 10 || ch == -1) break;
    buffer.print( (char) ch );
  }
  return new ASCIIString(buffer.to_string());
}

//=============================================================================
//  JogCmd
//=============================================================================
JogTypeInfo* JogCmd::as_type()
{
  JogTypeInfo* possible_type = reinterpret_as_type();
  if (possible_type && possible_type->is_type()) return possible_type;
  return NULL;
}

JogTypeInfo* JogCmd::require_value()
{
  JogTypeInfo* result = type();
  if (result == NULL) throw error( "Expression must result in a value." );
  return result;
}

Ref<JogCmd> JogCmd::discarding_result()
{
  JogTypeInfo* result_type = type();

  if (result_type == NULL) return this;

  if (result_type->is_reference()) return new JogCmdDiscardRefResult(t,this);

  return new JogCmdDiscardDataResult(t,this);
}

Ref<JogCmd> JogCmd::cast_to_type( JogTypeInfo* to_type )
{
  JogTypeInfo* cur_type = type();

  if (cur_type == NULL)
  {
    throw t->error( "Expression must result in a value." );
  }

  if (cur_type == to_type) return this;

  if (to_type->is_primitive())
  {
    if (cur_type->is_primitive())
    {
      if (to_type == jog_type_manager.type_boolean && cur_type != jog_type_manager.type_boolean)
      {
        throw t->error( "Cannot cast a boolean value to a non-boolean value." );
      }

      if (to_type != jog_type_manager.type_boolean && cur_type == jog_type_manager.type_boolean)
      {
        throw t->error( "Cannot cast a non-boolean value to a boolean value." );
      }

      return new JogCmdCast( t, this, to_type );
    }
  }


  throw t->error( "TODO: finish Cmd::cast_to_type()" );
}

JogTypeInfo* JogCmd::require_reference()
{
  JogTypeInfo* this_type = type();
  if (this_type == NULL || (this_type->qualifiers & JOG_QUALIFIER_REFERENCE) == 0)
  {
    throw error( "Object reference required." );
  }
  return this_type;
}

JogTypeInfo* JogCmd::require_instance_of( JogTypeInfo* base_type )
{
  JogTypeInfo* this_type = require_reference();
  if ( !this_type->instance_of(base_type) )
  {
    StringBuilder buffer;
    buffer.print( "'" );
    base_type->name->print(buffer);
    buffer.print( "' reference required." );
    throw error( (const char*) buffer.to_string() );
  }
  return this_type;
}

JogTypeInfo* JogCmd::require_primitive()
{
  JogTypeInfo* this_type = type();
  if (this_type == NULL || (this_type->qualifiers & JOG_QUALIFIER_PRIMITIVE) == 0)
  {
    throw error( "Primitive value required." );
  }
  return this_type;
}


//=============================================================================
//  JogVM
//=============================================================================
void JogVM::parse( string filename )
{
  printf( "Parsing %s\n", filename.c_str() );
  Ref<JogParser> parser = new JogParser(filename.c_str());
  JogTypeInfo* type = parser->parse_type_def();
  while (type)
  {
    types.add( type );
    type = parser->parse_type_def();
  }
}

void JogVM::compile()
{
  jog_type_manager.type_object = jog_type_manager.must_find_type("Object");
  jog_type_manager.type_string = jog_type_manager.must_find_type("String");

  for (int i=0; i<types.count; ++i)
  {
    types[i]->resolve();
  }
}

void JogVM::run( const char* main_class_name )
{
  JogTypeInfo* main_class = JogTypeInfo::find(main_class_name);
  if (main_class == NULL)
  {
    StringBuilder buffer;
    buffer.print( "Main class '" );
    buffer.print( main_class_name );
    buffer.print( "' not found." );
    Ref<JogError> err = new JogError( (const char*) buffer.to_string() );
    throw err;
  }

//main_class->print_members();  //DEBUG

  add_native_handler( "PrintWriter::print(double)", PrintWriter__print__double );
  add_native_handler( "PrintWriter::print(int)", PrintWriter__print__int );
  add_native_handler( "PrintWriter::print(boolean)", PrintWriter__print__boolean );
  add_native_handler( "PrintWriter::print(char)", PrintWriter__print__char );

  // Call static initializer blocks
  for (int i=0; i<types.count; ++i)
  {
    JogTypeInfo* type = types[i];
    for (int j=0; j<type->static_initializers.count; ++j)
    {
      JogMethodInfo* m = *(type->static_initializers[j]);
      Ref<JogToken>  t = m->t;
      Ref<JogCmd> cmd = new JogCmdClassCall( t, m, NULL, NULL );
      push(*cmd);
      execute();
    }
  }

  JogRef main_object = main_class->create_instance(this);
  if ( *(main_class->call_init_object) )
  {
    push( main_object );
    push( *(main_class->call_init_object) );
    execute();
    /*
    Ref<JogToken>  t = main_class->m_init_object->t;
    Ref<JogCmd> cmd = new JogCmdStaticCall( t,
        *(main_class->m_init_object), new JogCmdObjectRef(t,main_object), NULL );
    push(*cmd);
    execute();
    */
  }

  call_void_method( main_object, "<init>()" );
}

void JogVM::add_native_handler( Ref<JogString> signature, JogNativeMethodHandler handler )
{
  native_methods[signature] = handler;
}

void JogVM::force_garbage_collection()
{
printf("GC\n");
  // Delete unused objects at head of list.
  while (all_objects && !all_objects->reference_count)
  {
    JogObject* next = all_objects->next_object;
    cur_object_bytes -= all_objects->total_object_bytes();
    delete all_objects;
    all_objects = next;
  }

  // Delete unused objects in body of list.
  if (all_objects)
  {
    JogObject* prev = all_objects;
    JogObject* cur = all_objects->next_object;
    while (cur)
    {
      if (cur->reference_count)
      {
        prev = cur;
        cur = cur->next_object;
      }
      else
      {
        JogObject* next = cur->next_object;
        cur_object_bytes -= cur->total_object_bytes();
        delete cur;
        prev->next_object = next;
        cur = next;
      }
    }
  }
}

void JogVM::delete_all_objects()
{
  // Release any objects on reference stack
  while (ref_stack_ptr != ref_stack_limit) *(ref_stack_ptr++) = NULL;

  while (all_objects)
  {
    JogObject* next = all_objects->next_object;
    delete all_objects;
    all_objects = next;
  }
}

void JogVM::register_object( JogObject* obj, int byte_size )
{
  obj->next_object = all_objects;
  all_objects = obj;
  cur_object_bytes += byte_size;
  if (cur_object_bytes > max_object_bytes)
  {
    force_garbage_collection();
    if (cur_object_bytes > max_object_bytes)
    {
      Ref<JogError> err = new JogError("Out of allotted memory.");
      throw err;
    }
  }
}

void JogVM::push_frame( JogMethodInfo* method_info )
{
  // Set frame registers to point just before start of object
  // context and parameters.
  (--frame_ptr)->init( instruction_stack_ptr, 
      data_stack_ptr + method_info->param_data_count, 
      ref_stack_ptr + method_info->param_ref_count,
      method_info );

  data_stack_ptr -= method_info->local_data_count;
  ref_stack_ptr  -= method_info->local_ref_count;
}

//=============================================================================
//  JogTypeInfo
//=============================================================================
JogTypeInfo* JogTypeInfo::create( Ref<JogToken> t, int qualifiers, Ref<JogString> name )
{
  JogTypeLookup::iterator entry = jog_type_manager.type_lookup.find(name);

  if (entry == jog_type_manager.type_lookup.end())
  {
    JogTypeInfo* type = new JogTypeInfo();
    type->t = t;
    type->name = name;
    type->qualifiers = qualifiers;
    jog_type_manager.type_lookup[name] = type;
    return type;
  }
  else
  {
    if (entry->second->qualifiers == 0)
    {
      // Only referenced before, not defined - still good.
      JogTypeInfo* type = *entry->second;
      type->t = t;
      type->qualifiers = qualifiers;
      return type;
    }
    else
    {
      // Already defined
      JogTypeInfo* type = *entry->second;

      StringBuilder buffer;
      buffer.print( "Type " );
      buffer.print( type->name->to_ascii()->data );
      buffer.print( " was already defined at line " );
      buffer.print( type->t->line );

      if (type->t->reader->filename == t->reader->filename)
      {
        buffer.print(".");
      }
      else
      {
        buffer.print( " of \"" );
        buffer.print( type->t->reader->filename->data );
        buffer.print( "." );
      }
      Ref<ASCIIString> error_mesg = new ASCIIString( buffer.to_string() );
      throw entry->second->t->error( error_mesg );
    }
  }
}

JogTypeInfo* JogTypeInfo::create( Ref<JogToken> t, int qualifiers, const char* name )
{
  return create( t, qualifiers, new JogString(name) );
}

JogTypeInfo* JogTypeInfo::reference( Ref<JogToken> t, Ref<JogString> name )
{
  JogTypeLookup::iterator entry = jog_type_manager.type_lookup.find(name);

  if (entry == jog_type_manager.type_lookup.end())
  {
    JogTypeInfo* type = new JogTypeInfo();
    type->t = t;
    type->name = name;
    type->qualifiers = 0;
    jog_type_manager.type_lookup[name] = type;
    return type;
  }
  else if (entry->second == jog_type_manager.type_void)
  {
    return NULL;
  }
  else
  {
    return *(entry->second);
  }
}

JogTypeInfo* JogTypeInfo::find( Ref<JogString> name )
{
  JogTypeLookup::iterator entry = jog_type_manager.type_lookup.find(name);

  if (entry == jog_type_manager.type_lookup.end())
  {
    return NULL;
  }
  else
  {
    return *(entry->second);
  }
}

JogRef JogTypeInfo::create_array( JogVM* vm, int count )
{
  int obj_size = (sizeof(JogObject) - 8) + count * element_type->element_size;

  JogObject* obj = (JogObject*) new char[obj_size];
  memset( obj, 0, obj_size );
  obj->type = this;
  obj->count = count;

  JogRef result((JogObject*)obj);
  vm->register_object(obj,obj_size);

  return result;
}

bool JogTypeInfo::instance_of( JogTypeInfo* base_type )
{
  // Same type
  if (this == base_type) return true;

  // Everything is instanceof Object.
  if (base_type == jog_type_manager.type_object) return true;

  // 'null' is instanceof anything.
  if (this == jog_type_manager.type_null) return true;

  if (base_class && base_class->instance_of(base_type)) return true;

  for (int i=0; i<interfaces.count; ++i)
  {
    if (interfaces[i]->instance_of(base_type)) return true;
  }

  return false;
}

bool JogTypeInfo::is_boolean()
{
  return (this == jog_type_manager.type_boolean);
}

bool JogTypeInfo::is_real()
{
  return (this == jog_type_manager.type_real64 || this == jog_type_manager.type_real32);
}

bool JogTypeInfo::is_integer()
{
  return (this == jog_type_manager.type_int64 || this == jog_type_manager.type_int32
      || this == jog_type_manager.type_int16 || this == jog_type_manager.type_int8
      || this == jog_type_manager.type_char);
}


int  JogTypeInfo::precision()
{
  if (is_primitive())
  {
    if (this == jog_type_manager.type_real64) return 7;
    if (this == jog_type_manager.type_real32) return 6;
    if (this == jog_type_manager.type_int64)  return 5;
    if (this == jog_type_manager.type_int32)  return 4;
    if (this == jog_type_manager.type_int16)  return 3;
    if (this == jog_type_manager.type_char)   return 2;
    if (this == jog_type_manager.type_int8)   return 1;
  }
  return 0;
}

bool JogTypeInfo::is_compatible_with( JogTypeInfo* other )
{
  if (this == other) return true;

  // TODO: primitive boxing

  if (this->is_reference() ^ other->is_reference()) return false;

  if (is_primitive())
  {
    if (other == jog_type_manager.type_real64) return true;
    if (this  == jog_type_manager.type_real64) return false;
    if (other == jog_type_manager.type_real32) return true;
    if (this  == jog_type_manager.type_real32) return false;
    if (other == jog_type_manager.type_int64) return true;
    if (this  == jog_type_manager.type_int64) return false;
    if (other == jog_type_manager.type_int32) return true;
    if (this  == jog_type_manager.type_int32) return false;
    return (other == jog_type_manager.type_int16 && this == jog_type_manager.type_int8);
  }

  throw t->error( "TODO: is_compatible_with + instance_of for class types" );
}

void JogTypeInfo::organize()
{
  if (organized) return;
  organized = true;

  element_size = sizeof(void*);

  if (qualifiers == 0)
  {
    if (name->get(-1) == '[')
    {
      element_type = JogTypeInfo::reference( t, name->substring(0,name->count-2) );
      element_type->organize();
      qualifiers = JOG_QUALIFIER_REFERENCE;
    }
    else
    {
      StringBuilder buffer;
      buffer.print( "Reference to undefined type '" );
      buffer.print( name->to_ascii()->data );
      buffer.print( "'." );
      throw t->error( buffer.to_string() );
    }
  }

  // Inherit base class properties and methods.
  if ( !base_class && this != jog_type_manager.type_object )
  {
    base_class = jog_type_manager.type_object;
  }

  if (base_class) base_class->organize();

  RefList<JogMethodInfo> original_class_methods;
  for (int i=0; i<class_methods.count; ++i) original_class_methods.add(class_methods[i]);
  class_methods.clear();

  RefList<JogMethodInfo> original_methods;
  for (int i=0; i<methods.count; ++i) original_methods.add(methods[i]);
  methods.clear();

  if (base_class)
  {
    if (base_class->instance_of(this))
    {
      throw t->error( "Illegal cyclic inheritance." );
    }

    for (int i=base_class->properties.count-1; i>=0; --i)
    {
      properties.insert( base_class->properties[i] );
    }

    for (int i=0; i<base_class->class_methods.count; ++i)
    {
      add( base_class->class_methods[i] );
    }

    for (int i=0; i<base_class->methods.count; ++i)
    {
      add( base_class->methods[i] );
    }
  }

  for (int i=0; i<original_class_methods.count; ++i)
  {
    add( original_class_methods[i] );
  }

  for (int i=0; i<original_methods.count; ++i)
  {
    add( original_methods[i] );
  }

  for (int i=0; i<interfaces.count; ++i) interfaces[i]->organize();

  class_data_count = 0;
  data_count = 0;

  for (int i=0; i<class_properties.count; ++i)
  {
    class_properties[i]->type->organize();
    class_properties[i]->index = i;
    ++class_data_count;
  }

  for (int i=0; i<properties.count; ++i)
  {
    properties[i]->type->organize();
    properties[i]->index = i;
    ++data_count;
  }

  if (class_data_count)
  {
    class_data = new JogInt64[class_data_count];
    memset( class_data, 0, class_data_count*8 );
  }

  object_size = sizeof(JogObject) + (data_count - 1)*8;

  for (int i=0; i<class_methods.count; ++i)
  {
    class_methods[i]->organize();
      // Ensures types are defined, sets up class hierarchy, and creates method
      // signatures.
  }

  for (int i=0; i<methods.count; ++i)
  {
    methods[i]->organize();
      // Ensures types are defined, sets up class hierarchy, and creates method
      // signatures.
  }

  // Store class property initialization values in first (currently empty)
  // static initializer block.
  for (int i=0; i<class_properties.count; ++i)
  {
    JogPropertyInfo* p = *(class_properties[i]);
    if ( *(p->initial_value) )
    {
      static_initializers[0]->statements->add(
          new JogCmdAssign(
            p->t,
            new JogCmdIdentifier(p->t,p->name),
            p->initial_value
          )
        );
    }
  }

  if (is_reference())
  {
    // Add init_object for initial property value assignments.
    JogMethodInfo* m = NULL;
    for (int i=0; i<properties.count; ++i)
    {
      JogPropertyInfo* p = *(properties[i]);
      if ( *(p->initial_value) )
      {
        if ( !m )
        {
          m = new JogMethodInfo( t, JOG_QUALIFIER_PUBLIC, 
                this, NULL, new JogString("init_object") );
        }

        m->statements->add(
            new JogCmdAssign(
              p->t,
              new JogCmdIdentifier(p->t,p->name),
              p->initial_value
            )
          );
        m->organize();
      }
    }
    if (m)
    {
      m_init_object = m;
      call_init_object = new JogCmdCallInitObject( m->t, m );
    }

    // Add a default constructor if necessary
    Ref<JogString> ctor_name = new JogString("<init>");
    if ( !methods_by_name.contains(ctor_name) )
    {
      methods_by_name[ctor_name] = new ArrayList<JogMethodInfo*>();

      Ref<JogMethodInfo> m = new JogMethodInfo( t, 
          JOG_QUALIFIER_PUBLIC|JOG_QUALIFIER_CONSTRUCTOR, 
          this, NULL, ctor_name );
      add(m);
    }
  }
}

void JogTypeInfo::add( Ref<JogMethodInfo> m )
{
  m->organize();

  if (m->is_constructor() && m->is_inherited(this)) return;

  if (m->is_static())
  {
    if (class_methods_by_signature.contains(m->full_signature))
    {
      if (m->is_inherited(this)) return;

      JogMethodInfo* existing = class_methods_by_signature[m->full_signature];
      if ( !existing->is_inherited(this) )
      {
        StringBuilder buffer;
        buffer.print( "A class method with the signature \"" );
        m->signature->print(buffer);
        buffer.print( "\" already exists in type \"" );
        name->print(buffer);
        buffer.print( "\"." );
        throw m->t->error( buffer.to_string() );
      }
    }

    class_methods.add(m);
    class_methods_by_signature[m->signature] = *m;

    if ( !class_methods_by_name.contains(m->name) )
    {
      class_methods_by_name[m->name] = new ArrayList<JogMethodInfo*>();
    }
    class_methods_by_name[m->name]->add(*m);
  }
  else
  {
    dispatch_table.ensure_count(m->dispatch_id+1);

    JogMethodInfo* existing = dispatch_table[m->dispatch_id];
    if (existing)
    {
      if ( ((m->return_type!=NULL) ^ (existing->return_type!=NULL))
          || (m->return_type 
            && (m->return_type->is_primitive() ^ existing->return_type->is_primitive()))
          || (m->return_type && !m->return_type->instance_of(existing->return_type)) )
      {
        throw m->t->error( "Return type incompatible with inherited method." );
      }

      if ( !existing->is_inherited(this) )
      {
        throw m->t->error( "A method with this signature already exists." );
      }

      methods.replace(existing,m);
    }
    else
    {
      methods.add(m);
    }

    dispatch_table[m->dispatch_id] = *m;

    methods_by_signature[m->signature] = *m;

    if ( !methods_by_name.contains(m->name) )
    {
      methods_by_name[m->name] = new ArrayList<JogMethodInfo*>();
    }
    methods_by_name[m->name]->add(*m);
  }
}

void JogTypeInfo::prep()
{
  if (prepped) return;
  prepped = true;

  organize();

  // Add properties to lookup tables.
  for (int i=0; i<class_properties.count; ++i)
  {
    JogPropertyInfo* p = *(class_properties[i]);
    if (class_properties_by_name.contains(p->name))
    {
      StringBuilder buffer;
      buffer.print( "A class property named \"" );
      p->name->print(buffer);
      buffer.print( "\" already exists in type \"" );
      name->print(buffer);
      buffer.print( "\"." );
      throw p->t->error( buffer.to_string() );
    }
    class_properties_by_name[p->name] = p;
  }

  for (int i=0; i<properties.count; ++i)
  {
    JogPropertyInfo* p = *(properties[i]);
    if (properties_by_name.contains(p->name))
    {
      StringBuilder buffer;
      buffer.print( "A property named \"" );
      p->name->print(buffer);
      buffer.print( "\" already exists in type \"" );
      name->print(buffer);
      buffer.print( "\"." );
      throw p->t->error( buffer.to_string() );
    }
    properties_by_name[p->name] = p;
  }

}


//=============================================================================
//  JogTypeManager
//=============================================================================
JogTypeManager jog_type_manager;


Ref<JogCmd> JogCmdLiteralReal64::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_real64) return this;

  if (to_type == jog_type_manager.type_real32)
  {
    return new JogCmdLiteralReal32( t, (float) value );
  }

  if (to_type == jog_type_manager.type_int64)
  {
    return new JogCmdLiteralInt64( t, (JogInt64) value );
  }

  if (to_type == jog_type_manager.type_int32)
  {
    return new JogCmdLiteralInt32( t, (int) value );
  }

  if (to_type == jog_type_manager.type_int16)
  {
    return new JogCmdLiteralInt16( t, (JogInt16) value );
  }

  if (to_type == jog_type_manager.type_int8)
  {
    return new JogCmdLiteralInt8( t, (JogInt8) value );
  }

  if (to_type == jog_type_manager.type_char)
  {
    return new JogCmdLiteralChar( t, (JogChar) value );
  }
  StringBuilder buffer;
  buffer.print( "Can't cast type 'double' to type " );
  buffer.print( to_type->name->to_ascii()->data );
  buffer.print( "." );
  throw error( buffer.to_string() );
}

Ref<JogCmd> JogCmdLiteralReal32::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_real32) return this;

  if (to_type == jog_type_manager.type_real64)
  {
    return new JogCmdLiteralReal64( t, value );
  }

  if (to_type == jog_type_manager.type_int64)
  {
    return new JogCmdLiteralInt64( t, (JogInt64) value );
  }

  if (to_type == jog_type_manager.type_int32)
  {
    return new JogCmdLiteralInt32( t, (int) value );
  }

  if (to_type == jog_type_manager.type_int16)
  {
    return new JogCmdLiteralInt16( t, (JogInt16) value );
  }

  if (to_type == jog_type_manager.type_int8)
  {
    return new JogCmdLiteralInt8( t, (JogInt8) value );
  }

  if (to_type == jog_type_manager.type_char)
  {
    return new JogCmdLiteralChar( t, (JogChar) value );
  }

  StringBuilder buffer;
  buffer.print( "Can't cast type 'float' to type " );
  buffer.print( to_type->name->to_ascii()->data );
  buffer.print( "." );
  throw error( buffer.to_string() );
}

Ref<JogCmd> JogCmdLiteralInt64::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_int64) return this;

  if (to_type == jog_type_manager.type_real64)
  {
    return new JogCmdLiteralReal64( t, (double) value );
  }

  if (to_type == jog_type_manager.type_real32)
  {
    return new JogCmdLiteralReal32( t, (float) value );
  }

  if (to_type == jog_type_manager.type_int32)
  {
    return new JogCmdLiteralInt32( t, (JogInt32) value );
  }

  if (to_type == jog_type_manager.type_int16)
  {
    return new JogCmdLiteralInt16( t, (JogInt16) value );
  }

  if (to_type == jog_type_manager.type_int8)
  {
    return new JogCmdLiteralInt8( t, (JogInt8) value );
  }

  if (to_type == jog_type_manager.type_char)
  {
    return new JogCmdLiteralChar( t, (JogChar) value );
  }

  StringBuilder buffer;
  buffer.print( "Can't cast type 'int' to type " );
  buffer.print( to_type->name->to_ascii()->data );
  buffer.print( "." );
  throw error( buffer.to_string() );
}

Ref<JogCmd> JogCmdLiteralInt32::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_int32) return this;

  if (to_type == jog_type_manager.type_real64)
  {
    return new JogCmdLiteralReal64( t, (double) value );
  }

  if (to_type == jog_type_manager.type_real32)
  {
    return new JogCmdLiteralReal32( t, (float) value );
  }

  if (to_type == jog_type_manager.type_int64)
  {
    return new JogCmdLiteralInt64( t, (JogInt64) value );
  }

  if (to_type == jog_type_manager.type_int16)
  {
    return new JogCmdLiteralInt16( t, (JogInt16) value );
  }

  if (to_type == jog_type_manager.type_int8)
  {
    return new JogCmdLiteralInt8( t, (JogInt8) value );
  }

  if (to_type == jog_type_manager.type_char)
  {
    return new JogCmdLiteralChar( t, (JogChar) value );
  }

  StringBuilder buffer;
  buffer.print( "Can't cast type 'int' to type " );
  buffer.print( to_type->name->to_ascii()->data );
  buffer.print( "." );
  throw error( buffer.to_string() );
}

Ref<JogCmd> JogCmdLiteralInt16::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_int16) return this;

  if (to_type == jog_type_manager.type_real64)
  {
    return new JogCmdLiteralReal64( t, (double) value );
  }

  if (to_type == jog_type_manager.type_real32)
  {
    return new JogCmdLiteralReal32( t, (float) value );
  }

  if (to_type == jog_type_manager.type_int64)
  {
    return new JogCmdLiteralInt64( t, (JogInt64) value );
  }

  if (to_type == jog_type_manager.type_int32)
  {
    return new JogCmdLiteralInt32( t, (JogInt32) value );
  }

  if (to_type == jog_type_manager.type_int8)
  {
    return new JogCmdLiteralInt8( t, (JogInt8) value );
  }

  if (to_type == jog_type_manager.type_char)
  {
    return new JogCmdLiteralChar( t, (JogChar) value );
  }

  StringBuilder buffer;
  buffer.print( "Can't cast type 'short' to type " );
  buffer.print( to_type->name->to_ascii()->data );
  buffer.print( "." );
  throw error( buffer.to_string() );
}

Ref<JogCmd> JogCmdLiteralInt8::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_int8) return this;

  if (to_type == jog_type_manager.type_real64)
  {
    return new JogCmdLiteralReal64( t, (double) value );
  }

  if (to_type == jog_type_manager.type_real32)
  {
    return new JogCmdLiteralReal32( t, (float) value );
  }

  if (to_type == jog_type_manager.type_int64)
  {
    return new JogCmdLiteralInt64( t, (JogInt64) value );
  }

  if (to_type == jog_type_manager.type_int32)
  {
    return new JogCmdLiteralInt32( t, (JogInt32) value );
  }

  if (to_type == jog_type_manager.type_int16)
  {
    return new JogCmdLiteralInt16( t, (JogInt16) value );
  }

  if (to_type == jog_type_manager.type_char)
  {
    return new JogCmdLiteralChar( t, (JogChar) value );
  }

  StringBuilder buffer;
  buffer.print( "Can't cast type 'int' to type " );
  buffer.print( to_type->name->to_ascii()->data );
  buffer.print( "." );
  throw error( buffer.to_string() );
}

Ref<JogCmd> JogCmdLiteralChar::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_char) return this;

  if (to_type == jog_type_manager.type_real64)
  {
    return new JogCmdLiteralReal64( t, (double) value );
  }

  if (to_type == jog_type_manager.type_real32)
  {
    return new JogCmdLiteralReal32( t, (float) value );
  }

  if (to_type == jog_type_manager.type_int64)
  {
    return new JogCmdLiteralInt64( t, (JogInt64) value );
  }

  if (to_type == jog_type_manager.type_int32)
  {
    return new JogCmdLiteralInt32( t, (JogInt32) value );
  }

  if (to_type == jog_type_manager.type_int16)
  {
    return new JogCmdLiteralInt16( t, (JogInt16) value );
  }

  if (to_type == jog_type_manager.type_int8)
  {
    return new JogCmdLiteralInt8( t, (JogInt8) value );
  }

  StringBuilder buffer;
  buffer.print( "Can't cast type 'int' to type " );
  buffer.print( to_type->name->to_ascii()->data );
  buffer.print( "." );
  throw error( buffer.to_string() );
}

Ref<JogCmd> JogCmdLiteralBoolean::cast_to_type( JogTypeInfo* to_type )
{
  if (to_type == jog_type_manager.type_boolean) return this;
  throw error( "A boolean cannot be cast to any other type." );
}

void JogCmd::require_boolean()
{
  if (type() != jog_type_manager.type_boolean)
  {
    throw error( "A boolean value is required." );
  }
}

JogTypeInfo* JogCmd::require_integer()
{
  JogTypeInfo* this_type = type();
  if (this_type == jog_type_manager.type_int64
    || this_type == jog_type_manager.type_int32
    || this_type == jog_type_manager.type_int16
    || this_type == jog_type_manager.type_int8
    || this_type == jog_type_manager.type_char)
  {
    return this_type;
  }
  throw error( "An integer value is required." );
}

JogTypeInfo* JogCmd::require_integer_or_boolean()
{
  JogTypeInfo* this_type = type();
  if (this_type == jog_type_manager.type_int64
    || this_type == jog_type_manager.type_int32
    || this_type == jog_type_manager.type_int16
    || this_type == jog_type_manager.type_int8
    || this_type == jog_type_manager.type_char
    || this_type == jog_type_manager.type_boolean)
  {
    return this_type;
  }
  throw error( "An integer or boolean value is required." );
}


//=============================================================================
//  JogVM
//=============================================================================
void JogVM::call_native( JogMethodInfo* m )
{
  if (m->native_handler == NULL)
  {
    m->native_handler = native_methods[m->full_signature];
    if (m->native_handler == NULL)
    {
      throw m->t->error( "Native method not implemented in virtual machine." );
    }
  }
  m->native_handler(this);
}

void JogVM::call_void_method( JogRef context, const char* signature )
{
  JogMethodInfo* m = context->type->methods_by_signature[signature];
  Ref<JogToken>  t = m->t;
  Ref<JogCmd> cmd = new JogCmdDynamicCall( t, m, new JogCmdObjectRef(t,context), NULL );

  push(*cmd);
  execute();
}


//=============================================================================
//  JogPropertyInfo
//=============================================================================
JogPropertyInfo::JogPropertyInfo( Ref<JogToken> t, int qualifiers, JogTypeInfo* type_context, 
    JogTypeInfo* type, Ref<JogString> name, Ref<JogCmd> initial_value )
  : t(t), qualifiers(qualifiers), index(-1), type_context(type_context), 
    type(type), name(name), initial_value(initial_value)
{
}

void JogPropertyInfo::print()
{
  type->print();
  printf(" ");
  name->print();
  if (*initial_value) 
  {
    printf("=");
    initial_value->print();
  }
  printf("\n");
}


//=============================================================================
//  JogLocalVarInfo
//=============================================================================
JogLocalVarInfo::JogLocalVarInfo( Ref<JogToken> t, JogTypeInfo* type, Ref<JogString> name )
    : t(t), type(type), name(name), index(-1)
{
}

void JogLocalVarInfo::print()
{
  type->print();
  printf(" ");
  name->print();
}



//=============================================================================
//  JogMethodInfo
//=============================================================================
int JogMethodInfo::next_method_id = 1;

JogMethodInfo::JogMethodInfo( Ref<JogToken> t, int qualifiers, JogTypeInfo* type_context,
    JogTypeInfo* return_type, Ref<JogString> name )
  : t(t), qualifiers(qualifiers), type_context(type_context), return_type(return_type), 
    calls_super_constructor(false),
    name(name), native_handler(NULL), organized(false), resolved(false)
{
  statements = new JogStatementList(t);
  method_id = next_method_id++;
  dispatch_id = 0;
}

void JogMethodInfo::print()
{
  if (return_type)
  {
    return_type->print();
    printf(" ");
  }
  else
  {
    printf("void ");
  }
  name->print();
  printf("\n");
  printf("{\n");
  statements->print();
  printf("}\n\n");
}

bool JogMethodInfo::is_less_specific_than( JogMethodInfo* other )
{
  // Requires: 
  //   - Methods have the same name.
  //   - Method parameters are compatible.
  // Return types are ignored.
  if (parameters.count != other->parameters.count) return false;

  int count = parameters.count;
  int less_specific = 0;
  int more_specific = 0;
  for (int i=0; i<count; ++i)
  {
    JogTypeInfo* this_type  = parameters[i]->type;
    JogTypeInfo* other_type = other->parameters[i]->type;
    if (this_type != other_type)
    {
      if (this_type->is_compatible_with(other_type)) ++more_specific;
      else ++less_specific;
    }
  }

  return (less_specific && !more_specific);
}

bool JogMethodInfo::is_inherited( JogTypeInfo* type )
{
  return type_context != type;
}


void JogMethodInfo::organize()
{
  if (organized) return;
  organized = true;

  if (return_type) return_type->organize();
  {
    UnicodeStringBuilder buffer;
    name->print(buffer);
    buffer.print('(');

    for (int i=0; i<parameters.count; ++i)
    {
      parameters[i]->type->organize();
      if (i > 0) buffer.print(',');
      parameters[i]->type->name->print(buffer);
    }
    buffer.print(')');
    signature = new JogString(buffer.to_string());
  }
  {
    UnicodeStringBuilder buffer;
    type_context->name->print(buffer);
    buffer.print("::");
    signature->print(buffer);
    full_signature = new JogString(buffer.to_string());
  }

  dispatch_id = jog_type_manager.dispatch_id_lookup[signature];
}

void PrintWriter__print__double( JogVM* vm )
{
  printf("%lf",vm->pop_double());
}

void PrintWriter__print__int( JogVM* vm )
{
  printf("%d",vm->pop_int());
}

void PrintWriter__print__boolean( JogVM* vm )
{
  if (vm->pop_int()) printf("true");
  else               printf("false");
}

void PrintWriter__print__char( JogVM* vm )
{
  putchar( vm->pop_int() );
}

