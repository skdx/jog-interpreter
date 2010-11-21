#include "jog.h"

void JogCmd::on_push( JogVM* vm ) 
{ 
  printf("\nOn line %d\n", node_type() );
  print();
  printf("\n");
  throw error( "[Internal] Command with unsupported on_push()." );
}

void JogCmd::execute( JogVM* vm ) 
{ 
  printf("\nOn line %d\n", node_type() );
  print();
  printf("\n");
  throw error( "[Internal] Command with unsupported execute()." );
}

void JogCmdList::on_push( JogVM* vm )
{
}

void JogCmdList::execute( JogVM* vm )
{
  int  statement_index = vm->execution_state();

  if (statement_index < commands.count)
  {
    vm->run_this_again();
    vm->push( *(commands.data[statement_index]) );
  }
}

void JogCmdLiteralReal64::on_push( JogVM* vm ) { }

void JogCmdLiteralReal64::execute( JogVM* vm ) { vm->push( *((JogInt64*) &value) ); }

void JogCmdLiteralReal32::on_push( JogVM* vm ) { }

void JogCmdLiteralReal32::execute( JogVM* vm ) 
{ 
  double d = value;
  vm->push( *((JogInt64*) &d) ); 
}

void JogCmdLiteralInt64::on_push( JogVM* vm ) { }

void JogCmdLiteralInt64::execute( JogVM* vm ) { vm->push( value ); }

void JogCmdLiteralInt32::on_push( JogVM* vm ) { }

void JogCmdLiteralInt32::execute( JogVM* vm ) { vm->push( value ); }

void JogCmdLiteralInt16::on_push( JogVM* vm ) { }

void JogCmdLiteralInt16::execute( JogVM* vm ) { vm->push( value ); }

void JogCmdLiteralInt8::on_push( JogVM* vm ) { }

void JogCmdLiteralInt8::execute( JogVM* vm ) { vm->push( value ); }

void JogCmdLiteralChar::on_push( JogVM* vm ) { }

void JogCmdLiteralChar::execute( JogVM* vm ) { vm->push( value ); }

void JogCmdLiteralBoolean::on_push( JogVM* vm ) { }

void JogCmdLiteralBoolean::execute( JogVM* vm ) { vm->push( value ); }

void JogCmdLiteralString::on_push( JogVM* vm ) { }

void JogCmdLiteralString::execute( JogVM* vm )
{
  if ( !*runtime_object )
  {
    runtime_object = vm->create_string( (JogChar*) value->data, value->count );
  }
  vm->push( runtime_object );
}

void JogCmdBlock::on_push( JogVM* vm ) { }

void JogCmdBlock::execute( JogVM* vm )
{
  int  statement_index = vm->execution_state();
  RefList<JogCmd>* commands = &statements->commands;
  int count = commands->count;

  if (statement_index < count)
  {
    vm->run_this_again();
    vm->push( *(*commands)[statement_index] );
  }
}

void JogCmdIf::on_push( JogVM* vm ) 
{ 
  vm->push( *expression );
}

void JogCmdIf::execute( JogVM* vm ) 
{
  if (vm->pop_data())
  {
    vm->push( *body );
  }
  else if (*else_body)
  {
    vm->push(*else_body);
  }
}

void JogCmdWhile::on_push( JogVM* vm ) 
{ 
  vm->push( *expression );
}

void JogCmdWhile::execute( JogVM* vm ) 
{
  if (vm->pop_data())
  {
    vm->run_this_again();
    vm->push( *expression );
    vm->push( *body );
  }
}

void JogCmdFor::on_push( JogVM* vm ) 
{ 
  vm->push( *condition );
  vm->push( *initialization );
}

void JogCmdFor::execute( JogVM* vm ) 
{
  int  state = vm->execution_state();

  if (state&1)
  {
    vm->run_this_again();
    vm->push( *condition );
    vm->push( *var_mod );
  }
  else
  {
    if (vm->pop_data())
    {
      vm->run_this_again();
      vm->push( *body );
    }
  }
}

void JogCmdReturnVoid::on_push( JogVM* vm )
{
}

void JogCmdReturnVoid::execute( JogVM* vm )
{
  vm->pop_frame();
}

void JogCmdUnary::on_push( JogVM* vm )
{
  vm->push( *operand );
}

void JogCmdBinary::on_push( JogVM* vm )
{
  vm->push( *rhs );
  vm->push( *lhs );
}

void JogCmdLogicalOr::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a || b );
}

void JogCmdLogicalAnd::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a && b );
}

void JogCmdBitwiseOr::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a | b );
}

void JogCmdBitwiseXor::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a ^ b );
}

void JogCmdBitwiseAnd::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a & b );
}

void JogCmdShift::on_push( JogVM* vm )
{
  vm->push( *shift_amount );
  vm->push( *operand );
}

void JogCmdNewObject::on_push( JogVM* vm )
{

  int count = args->commands.count;
  Ref<JogCmd>* cmd_ptr = args->commands.data + count;
  ++count;

  while (--count) vm->push( **(--cmd_ptr) );

  vm->push( this, -1 );  // second copy of this cmd with special state
}

void JogCmdNewObject::execute( JogVM* vm )
{
  int  execution_state = vm->execution_state();

  if (execution_state == -1)
  {
    JogRef new_obj = of_type->create_instance(vm);
    vm->push( new_obj );  // final result
    vm->push( new_obj );  // consumed by constructor

    if ( *(of_type->call_init_object) )
    {
      vm->push( new_obj );  // consumed by init_object()
      vm->push( *(of_type->call_init_object) );  // call init_object()
    }
    return;
  }

  RefList<JogCmd>* commands = &method_info->statements->commands;
  int count = commands->count;

  if (execution_state == 0)
  {
    vm->push_frame( method_info );
  }

  if (execution_state < count)
  {
    vm->run_this_again();
    vm->push( *(*commands)[execution_state] );
  }
  else
  {
    vm->pop_frame();
  }
}

void JogCmdNewArray::on_push( JogVM* vm )
{
  vm->push( *size_expr );
}

void JogCmdNewArray::execute( JogVM* vm )
{
  int count = vm->pop_int();
  if (count < 0)
  {
    throw error( "Illegal negative size." ); 
  }
  vm->push( of_type->create_array(vm,count) );
}

void JogCmdLiteralArray::on_push( JogVM* vm )
{
}

void JogCmdLiteralArray::execute( JogVM* vm )
{
  int execution_state = vm->execution_state();

  RefList<JogCmd>* commands = &terms->commands;
  int count = commands->count;

  if (execution_state == 0)
  {
    if (count > 0) vm->run_this_again();

    JogRef new_obj = of_type->create_array(vm,terms->count());
    vm->push( new_obj );  // final result, repeatedly fetched to add each term
    if (count > 0)
    {
      vm->push( *(*commands)[0] );
    }
    return;
  }

  if (execution_state < count)
  {
    vm->run_this_again();
  }

  // Add result of previous execution to array and
  // push next term on stack.
  if (of_type->element_type->is_reference())
  {
    // reference type
    JogRef element = vm->pop_ref();
    element->retain();
    ((JogObject**)(vm->peek_ref()->data))[execution_state-1] = *element;
  }
  else
  {
    // primitive type
    JogInt64 element = vm->pop_data();
    store_value( vm, execution_state-1, element );
  }

  if (execution_state < count)
  {
    vm->push( *(*commands)[execution_state] );
  }
}

void JogCmdLogicalNot::execute( JogVM* vm ) 
{
  if (vm->pop_data()) vm->push(0);
  else                vm->push(1);
}

void JogCmdBitwiseNot::execute( JogVM* vm ) 
{
  JogInt64 value = vm->pop_data();
  vm->push( ~value );
}

void JogCmdCastReal32ToReal64::execute( JogVM* vm )
{
  double d = vm->pop_data_as_Real64();
  vm->push( (float) d );
}

void JogCmdCastIntegerToReal64::execute( JogVM* vm )
{
  JogInt64 n = (JogInt64) vm->pop_data();
  vm->push( (double) n );
}

void JogCmdCastReal64ToReal32::execute( JogVM* vm )
{
  double d = vm->pop_data_as_Real64();
  vm->push( (float) d );
}

void JogCmdCastIntegerToReal32::execute( JogVM* vm )
{
  JogInt64 n = (JogInt64) vm->pop_data();
  vm->push( (float) n );
}

void JogCmdCastRealToInt64::execute( JogVM* vm )
{
  double d = vm->pop_data_as_Real64();
  vm->push( (JogInt64) d );
}

void JogCmdCastIntegerToInt64::execute( JogVM* vm )
{
  // no action
}

void JogCmdCastRealToInt32::execute( JogVM* vm )
{
  double d = vm->pop_data_as_Real64();
  vm->push( (int) d );
}

void JogCmdCastIntegerToInt32::execute( JogVM* vm )
{
  JogInt64 n = (JogInt64) vm->pop_data();
  vm->push( (int) n );
}

void JogCmdCastIntegerToInt16::execute( JogVM* vm )
{
  JogInt64 n = (JogInt64) vm->pop_data();
  vm->push( (int) n );
}

void JogCmdCastIntegerToInt8::execute( JogVM* vm )
{
  JogInt64 n = (JogInt64) vm->pop_data();
  vm->push( (int) n );
}

void JogCmdCastIntegerToChar::execute( JogVM* vm )
{
  JogInt64 n = (JogInt64) vm->pop_data();
  vm->push( (int) n );
}

void JogCmdWideningCast::execute( JogVM* vm )
{
  // no action
}

void JogCmdNarrowingCast::execute( JogVM* vm )
{
  JogObject* obj = vm->peek_ref().null_check(t);
  if ( !obj->type->instance_of(to_type) )
  {
    throw error ( "Illegal typecast." );
  }
}

void JogCmdAddReal64::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( a+b );
}

void JogCmdAddReal32::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (float) (a+b) );
}

void JogCmdAddInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a+b );
}

void JogCmdAddInt32::execute( JogVM* vm )
{
  int b = (int) vm->pop_data();
  int a = (int) vm->pop_data();
  vm->push( a+b );
}

void JogCmdSubReal64::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( a-b );
}

void JogCmdSubReal32::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (float) (a-b) );
}

void JogCmdSubInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a-b );
}

void JogCmdSubInt32::execute( JogVM* vm )
{
  int b = (int) vm->pop_data();
  int a = (int) vm->pop_data();
  vm->push( a-b );
}

void JogCmdMulReal64::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( a*b );
}

void JogCmdMulReal32::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (float) (a*b) );
}

void JogCmdMulInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a*b );
}

void JogCmdMulInt32::execute( JogVM* vm )
{
  int b = (int) vm->pop_data();
  int a = (int) vm->pop_data();
  vm->push( a*b );
}

void JogCmdDivReal64::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( a/b );
}

void JogCmdDivReal32::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (float) (a/b) );
}

void JogCmdDivInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  if ( !b ) throw error( "Integer divide by zero error during execution." );
  vm->push( a/b );
}

void JogCmdDivInt32::execute( JogVM* vm )
{
  int b = (int) vm->pop_data();
  int a = (int) vm->pop_data();
  if ( !b ) throw error( "Integer divide by zero error during execution." );
  vm->push( a/b );
}

void JogCmdModInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  if ( !b ) throw error( "Integer divide by zero error during execution." );
  vm->push( a%b );
}

void JogCmdModInt32::execute( JogVM* vm )
{
  int b = (int) vm->pop_data();
  int a = (int) vm->pop_data();
  if ( !b ) throw error( "Integer divide by zero error during execution." );
  vm->push( a%b );
}

void JogCmdAndInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a&b );
}

void JogCmdAndInt32::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (JogInt32)(a&b) );
}

void JogCmdOrInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a|b );
}

void JogCmdOrInt32::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (JogInt32)(a|b) );
}

void JogCmdXorInt64::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( a^b );
}

void JogCmdXorInt32::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (JogInt32)(a^b) );
}

void JogCmdEQReal::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (a==b)?1:0 );
}

void JogCmdEQInteger::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (a==b)?1:0 );
}

void JogCmdEQRef::execute( JogVM* vm )
{
  JogRef b = vm->pop_ref();
  JogRef a = vm->pop_ref();
  vm->push( (a==b)?1:0 );
}

void JogCmdNEReal::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (a!=b)?1:0 );
}

void JogCmdNEInteger::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (a!=b)?1:0 );
}

void JogCmdNERef::execute( JogVM* vm )
{
  JogRef b = vm->pop_ref();
  JogRef a = vm->pop_ref();
  vm->push( (a!=b)?1:0 );
}

void JogCmdLTReal::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (a<b)?1:0 );
}

void JogCmdLTInteger::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (a<b)?1:0 );
}

void JogCmdLEReal::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (a<=b)?1:0 );
}

void JogCmdLEInteger::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (a<=b)?1:0 );
}

void JogCmdGTReal::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (a>b)?1:0 );
}

void JogCmdGTInteger::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (a>b)?1:0 );
}

void JogCmdGEReal::execute( JogVM* vm )
{
  double b = vm->pop_data_as_Real64();
  double a = vm->pop_data_as_Real64();
  vm->push( (a>=b)?1:0 );
}

void JogCmdGEInteger::execute( JogVM* vm )
{
  JogInt64 b = vm->pop_data();
  JogInt64 a = vm->pop_data();
  vm->push( (a>=b)?1:0 );
}

void JogCmdLeftShiftInt64::execute( JogVM* vm )
{
  JogInt64 shift_amount = vm->pop_data();
  JogInt64 value = vm->pop_data();
  vm->push( value << shift_amount );
}

void JogCmdLeftShiftInt32::execute( JogVM* vm )
{
  int shift_amount = (int) vm->pop_data();
  int value = (int) vm->pop_data();
  vm->push( (int) (value << shift_amount) );
}

void JogCmdRightShiftInt64::execute( JogVM* vm )
{
  JogInt64 shift_amount = vm->pop_data();
  JogInt64 value = vm->pop_data();
  if (shift_amount > 0)
  {
    value = (value >> 1) & 0x7FFFffffFFFFffffLL;
    if (shift_amount > 1)
    {
      value >>= (shift_amount - 1);
    }
  }
  vm->push( value );
}

void JogCmdRightShiftInt32::execute( JogVM* vm )
{
  int shift_amount = (int) vm->pop_data();
  int value = (int) vm->pop_data();
  if (shift_amount > 0)
  {
    value = (value >> 1) & 0x7FFFffff;
    if (shift_amount > 1)
    {
      value >>= (shift_amount - 1);
    }
  }
  vm->push( (int) value );
}

void JogCmdRightXShiftInt64::execute( JogVM* vm )
{
  JogInt64 shift_amount = vm->pop_data();
  JogInt64 value = vm->pop_data();
  vm->push( value >> shift_amount );
}

void JogCmdRightXShiftInt32::execute( JogVM* vm )
{
  int shift_amount = (int) vm->pop_data();
  int value = (int) vm->pop_data();
  vm->push( (int) (value >> shift_amount) );
}

void JogCmdNegateReal64::execute( JogVM* vm )
{
  double n = vm->pop_double();
  vm->push( -n );
}

void JogCmdNegateReal32::execute( JogVM* vm )
{
  float n = (float) vm->pop_double();
  vm->push( -n );
}

void JogCmdNegateInt64::execute( JogVM* vm )
{
  JogInt64 n = vm->pop_data();
  vm->push( -n );
}

void JogCmdNegateInt32::execute( JogVM* vm )
{
  int n = vm->pop_int();
  vm->push( -n );
}

void JogCmdNegateInt16::execute( JogVM* vm )
{
  int n = vm->pop_int();
  vm->push( (JogInt16) -n );
}

void JogCmdNegateInt8::execute( JogVM* vm )
{
  int n = vm->pop_int();
  vm->push( (JogInt8) -n );
}

void JogCmdNegateChar::execute( JogVM* vm )
{
  int n = vm->pop_int();
  vm->push( (char) -n );
}

void JogCmdThis::on_push( JogVM* vm ) { }

void JogCmdThis::execute( JogVM* vm )
{
  vm->push(vm->frame_ptr->ref_stack_ptr[-1]);
}

void JogCmdArraySize::on_push( JogVM* vm )
{
  vm->push( *context );
}

void JogCmdArraySize::execute( JogVM* vm )
{
  JogObject* obj = vm->pop_ref().null_check(t);
  vm->push( obj->count );
}


void JogCmdNullRef::on_push( JogVM* vm ) { }

void JogCmdNullRef::execute( JogVM* vm )
{
  vm->push( JogRef() );
}

void JogCmdObjectRef::on_push( JogVM* vm ) { }

void JogCmdObjectRef::execute( JogVM* vm )
{
  vm->push(ref);
}

void JogCmdCallInitObject::on_push( JogVM* vm )
{
}

void JogCmdCallInitObject::execute( JogVM* vm )
{
  int  statement_index = vm->execution_state();
  if (statement_index == 0)
  {
    vm->push_frame( method_info );
  }

  RefList<JogCmd>* commands = &method_info->statements->commands;
  int count = commands->count;

  if (statement_index < count)
  {
    vm->run_this_again();
    vm->push( *(*commands)[statement_index] );
  }
  else
  {
    vm->pop_frame();
  }
}

void JogCmdStaticCall::on_push( JogVM* vm )
{
  if (*args)
  {
    int count = args->commands.count;
    Ref<JogCmd>* cmd_ptr = args->commands.data + count;
    ++count;

    while (--count) vm->push( **(--cmd_ptr) );
  }

  vm->push( *context );
}


void JogCmdStaticCall::execute( JogVM* vm )
{
  int  statement_index = vm->execution_state();
  if (statement_index == 0)
  {
    ((vm->ref_stack_ptr + (method_info->param_ref_count))[-1]).null_check(t);
    vm->push_frame( method_info );
    if (method_info->is_native())
    {
      vm->run_this_again();
      vm->call_native( method_info );
      return;
    }
  }
  else
  {
    if (method_info->is_native())
    {
      vm->pop_frame();
      return;
    }
  }

  RefList<JogCmd>* commands = &method_info->statements->commands;
  int count = commands->count;

  if (statement_index < count)
  {
    vm->run_this_again();
    vm->push( *(*commands)[statement_index] );
  }
  else if (method_info->return_type)
  {
    throw method_info->t->error( "Method does not return a value in all cases." );
  }
  else
  {
    vm->pop_frame();
  }
}

void JogCmdDynamicCall::execute( JogVM* vm )
{
  JogMethodInfo* m;
  int  statement_index = vm->execution_state();
  if (statement_index == 0)
  {
    JogObject* obj = ((vm->ref_stack_ptr + (method_info->param_ref_count))[-1]).null_check(t);
    m = obj->type->dispatch_table[method_info->dispatch_id];
    vm->push_frame( m );

    if (m->is_native())
    {
      vm->run_this_again();
      vm->call_native( m );
      return;
    }
  }
  else
  {
    m = vm->frame_ptr->called_method;
    if (m->is_native())
    {
      vm->pop_frame();
      return;
    }
  }

  RefList<JogCmd>* commands = &m->statements->commands;
  int count = commands->count;

  if (statement_index < count)
  {
    vm->run_this_again();
    vm->push( *(*commands)[statement_index] );
  }
  else if (method_info->return_type)
  {
    throw method_info->t->error( "Method does not return a value in all cases." );
  }
  else
  {
    vm->pop_frame();
  }
}

void JogCmdClassCall::on_push( JogVM* vm )
{
  if (*args)
  {
    int count = args->commands.count;
    Ref<JogCmd>* cmd_ptr = args->commands.data + count;
    ++count;

    while (--count) vm->push( **(--cmd_ptr) );
  }

  if (*context) vm->push( *context );
}

void JogCmdClassCall::execute( JogVM* vm )
{
  int  statement_index = vm->execution_state();
  if (statement_index == 0)
  {
    vm->push_frame(method_info);
    if (method_info->is_native())
    {
      vm->run_this_again();
      vm->call_native( method_info );
      return;
    }
  }
  else if (method_info->is_native())
  {
    vm->pop_frame();
    return;
  }

  RefList<JogCmd>* commands = &method_info->statements->commands;
  int count = commands->count;

  if (statement_index < count)
  {
    vm->run_this_again();
    vm->push( *(*commands)[statement_index] );
  }
  else if (method_info->return_type)
  {
    throw method_info->t->error( "Method does not return a value in all cases." );
  }
  else
  {
    vm->pop_frame();
  }
}

void JogCmdReturnData::on_push( JogVM* vm )
{
  vm->push(*operand);
}

void JogCmdReturnData::execute( JogVM* vm )
{
  JogInt64 result = vm->pop_data();
  vm->pop_frame();
  vm->push( result );
}

void JogCmdReturnRef::on_push( JogVM* vm )
{
  vm->push(*operand);
}

void JogCmdReturnRef::execute( JogVM* vm )
{
  JogRef result = vm->pop_ref();
  vm->pop_frame();
  vm->push( result );
}

void JogCmdDiscardDataResult::execute( JogVM* vm )
{
  //printf("Discarding %d\n", (int)(vm->pop_data()) );
  vm->pop_data();
}

void JogCmdDiscardRefResult::execute( JogVM* vm )
{
  vm->pop_ref();
}

void JogCmdReadClassProperty::on_push( JogVM* vm ) 
{ 
  if (*context) vm->push( *context );
}

void JogCmdReadClassPropertyData::execute( JogVM* vm )
{
  vm->push(var_info->type_context->class_data[var_info->index]);
}

void JogCmdReadClassPropertyRef::execute( JogVM* vm )
{
  vm->push( ((JogObject**)var_info->type_context->class_data)[var_info->index] );
}

void JogCmdWriteClassProperty::on_push( JogVM* vm ) 
{ 
  vm->push( *new_value );
  if (*context) vm->push( *context );
}

void JogCmdWriteClassPropertyData::execute( JogVM* vm )
{
  var_info->type_context->class_data[var_info->index] = vm->peek_data();
}

void JogCmdWriteClassPropertyRef::execute( JogVM* vm )
{
  JogObject* &location = ((JogObject**)var_info->type_context->class_data)[var_info->index];
  if (location) location->release();
  location = *(vm->peek_ref());
  if (location) location->retain();
}

void JogCmdReadProperty::on_push( JogVM* vm ) 
{ 
  vm->push( *context );
}

void JogCmdReadPropertyData::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  vm->push(context.null_check(t)->data[var_info->index]);
}

void JogCmdReadPropertyRef::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  vm->push( *((JogObject**)&(context.null_check(t)->data[var_info->index])) );
}

void JogCmdWriteProperty::on_push( JogVM* vm ) 
{ 
  vm->push( *new_value );
  vm->push( *context );
}

void JogCmdWritePropertyData::execute( JogVM* vm )
{
  JogInt64 new_value = vm->pop_data();
  JogRef context = vm->pop_ref();
  context.null_check(t)->data[var_info->index] = new_value;
  vm->push( new_value );
}

void JogCmdWritePropertyRef::execute( JogVM* vm )
{
  JogRef new_value = vm->pop_ref();
  JogRef context   = vm->pop_ref();
  JogObject** location = (JogObject**)&(context.null_check(t)->data[var_info->index]);
  if (*location) (*location)->release();
  *location = *new_value;
  if (*location) (*location)->retain();

  vm->push( new_value );
}

void JogCmdReadLocal::on_push( JogVM* vm ) { }

void JogCmdReadLocalData::execute( JogVM* vm )
{
  vm->push(vm->frame_ptr->data_stack_ptr[var_info->offset]);
}

void JogCmdReadLocalRef::execute( JogVM* vm )
{
  vm->push(vm->frame_ptr->ref_stack_ptr[var_info->offset]);
}

void JogCmdWriteLocal::on_push( JogVM* vm ) 
{ 
  vm->push( *new_value );
}

void JogCmdWriteLocalData::execute( JogVM* vm )
{
  vm->frame_ptr->data_stack_ptr[var_info->offset] = vm->peek_data();
}

void JogCmdWriteLocalRef::execute( JogVM* vm )
{
  vm->frame_ptr->ref_stack_ptr[var_info->offset] = vm->peek_ref();
}

//--------------------------------------------------------------------

void JogCmdPostStepLocalReal64::execute( JogVM* vm )
{
  double& local = ((double*)vm->frame_ptr->data_stack_ptr)[var_info->offset];
  vm->push( local );
  local += modifier;
}

void JogCmdPostStepLocalReal32::execute( JogVM* vm )
{
  double& local = ((double*)vm->frame_ptr->data_stack_ptr)[var_info->offset];
  vm->push( local );
  local = (float)(local += modifier);
}

void JogCmdPostStepLocalInt64::execute( JogVM* vm )
{
  JogInt64& local = vm->frame_ptr->data_stack_ptr[var_info->offset];
  vm->push( local );
  local += modifier;
}

void JogCmdPostStepLocalInt32::execute( JogVM* vm )
{
  JogInt64& local = vm->frame_ptr->data_stack_ptr[var_info->offset];
  vm->push( local );
  local = (JogInt32)(local + modifier);
}

void JogCmdPostStepLocalInt16::execute( JogVM* vm )
{
  JogInt64& local = vm->frame_ptr->data_stack_ptr[var_info->offset];
  vm->push( local );
  local = (JogInt16)(local + modifier);
}

void JogCmdPostStepLocalInt8::execute( JogVM* vm )
{
  JogInt64& local = vm->frame_ptr->data_stack_ptr[var_info->offset];
  vm->push( local );
  local = (JogInt8)(local + modifier);
}

void JogCmdPostStepLocalChar::execute( JogVM* vm )
{
  JogInt64& local = vm->frame_ptr->data_stack_ptr[var_info->offset];
  vm->push( local );
  local = (JogChar)(local + modifier);
}

//--------------------------------------------------------------------

void JogCmdPreStepClassProperty::on_push( JogVM* vm )
{
  if (*context) vm->push(*context);
}

void JogCmdPreStepClassPropertyReal64::execute( JogVM* vm )
{
  double& v = ((double*)var_info->type_context->class_data)[var_info->index];
  vm->push( v += modifier );
}

void JogCmdPreStepClassPropertyReal32::execute( JogVM* vm )
{
  double& v = ((double*)var_info->type_context->class_data)[var_info->index];
  v = (float)(v += modifier);
  vm->push( v );
}

void JogCmdPreStepClassPropertyInt64::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  vm->push( v += modifier );
}

void JogCmdPreStepClassPropertyInt32::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  v = (JogInt32)(v + modifier);
  vm->push( v );
}

void JogCmdPreStepClassPropertyInt16::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  v = (JogInt16)(v + modifier);
  vm->push( v );
}

void JogCmdPreStepClassPropertyInt8::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  v = (JogInt8)(v + modifier);
  vm->push( v );
}

void JogCmdPreStepClassPropertyChar::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  v = (JogChar)(v + modifier);
  vm->push( v );
}


void JogCmdPostStepClassProperty::on_push( JogVM* vm )
{
  if (*context) vm->push(*context);
}

void JogCmdPostStepClassPropertyReal64::execute( JogVM* vm )
{
  double& v = ((double*)var_info->type_context->class_data)[var_info->index];
  vm->push( v );
  v += modifier;
}

void JogCmdPostStepClassPropertyReal32::execute( JogVM* vm )
{
  double& v = ((double*)var_info->type_context->class_data)[var_info->index];
  vm->push( v );
  v = (float)(v += modifier);
}

void JogCmdPostStepClassPropertyInt64::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  vm->push( v );
  v += modifier;
}

void JogCmdPostStepClassPropertyInt32::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  vm->push( v );
  v = (JogInt32)(v + modifier);
}

void JogCmdPostStepClassPropertyInt16::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  vm->push( v );
  v = (JogInt16)(v + modifier);
}

void JogCmdPostStepClassPropertyInt8::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  vm->push( v );
  v = (JogInt8)(v + modifier);
}

void JogCmdPostStepClassPropertyChar::execute( JogVM* vm )
{
  JogInt64& v = var_info->type_context->class_data[var_info->index];
  vm->push( v );
  v = (JogChar)(v + modifier);
}

//--------------------------------------------------------------------

void JogCmdPreStepProperty::on_push( JogVM* vm )
{
  vm->push(*context);
}

void JogCmdPreStepPropertyReal64::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  double& v = ((double*)context->data)[var_info->index];
  vm->push( v += modifier );
}

void JogCmdPreStepPropertyReal32::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  double& v = ((double*)context->data)[var_info->index];
  v = (float)(v += modifier);
  vm->push( v );
}

void JogCmdPreStepPropertyInt64::execute( JogVM* vm )
{;
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];

  vm->push( v += modifier );
}

void JogCmdPreStepPropertyInt32::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  v = (JogInt32)(v + modifier);
  vm->push( v );
}

void JogCmdPreStepPropertyInt16::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  v = (JogInt16)(v + modifier);
  vm->push( v );
}

void JogCmdPreStepPropertyInt8::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  v = (JogInt8)(v + modifier);
  vm->push( v );
}

void JogCmdPreStepPropertyChar::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  v = (JogChar)(v + modifier);
  vm->push( v );
}


void JogCmdPostStepProperty::on_push( JogVM* vm )
{
  vm->push(*context);
}

void JogCmdPostStepPropertyReal64::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  double& v = ((double*)context->data)[var_info->index];
  vm->push( v );
  v += modifier;
}

void JogCmdPostStepPropertyReal32::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  double& v = ((double*)context->data)[var_info->index];
  vm->push( v );
  v = (float)(v += modifier);
}

void JogCmdPostStepPropertyInt64::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  vm->push( v );
  v += modifier;
}

void JogCmdPostStepPropertyInt32::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  vm->push( v );
  v = (JogInt32)(v + modifier);
}

void JogCmdPostStepPropertyInt16::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  vm->push( v );
  v = (JogInt16)(v + modifier);
}

void JogCmdPostStepPropertyInt8::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  vm->push( v );
  v = (JogInt8)(v + modifier);
}

void JogCmdPostStepPropertyChar::execute( JogVM* vm )
{
  JogRef context = vm->pop_ref();
  context.null_check(t);
  JogInt64& v = context->data[var_info->index];
  vm->push( v );
  v = (JogChar)(v + modifier);
}

//-----------------------------------------------------------------------------

void JogCmdArrayRead::on_push( JogVM* vm )
{
  vm->push( *index_expr );
  vm->push( *context );
}

void JogCmdArrayReadRef::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((JogObject**)array->data)[index] );
}

void JogCmdArrayReadReal64::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((double*)array->data)[index] );
}

void JogCmdArrayReadReal32::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((float*)array->data)[index] );
}

void JogCmdArrayReadInt64::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((JogInt64*)array->data)[index] );
}

void JogCmdArrayReadInt32::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((JogInt32*)array->data)[index] );
}

void JogCmdArrayReadInt16::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((JogInt16*)array->data)[index] );
}

void JogCmdArrayReadInt8::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((JogInt8*)array->data)[index] );
}

void JogCmdArrayReadChar::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((JogChar*)array->data)[index] );
}

void JogCmdArrayReadBoolean::execute( JogVM* vm )
{
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  vm->push( ((char*)array->data)[index] );
}

//-----------------------------------------------------------------------------

void JogCmdArrayWrite::on_push( JogVM* vm )
{
  vm->push( *new_value );
  vm->push( *index_expr );
  vm->push( *context );
}

void JogCmdArrayWriteRef::execute( JogVM* vm )
{
  JogRef value = vm->pop_ref();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);

  array->index_check(t,index);

  JogObject* &location = ((JogObject**)array->data)[index];
  if (location) location->release();
  location = *value;
  if (location) location->retain();

  vm->push( value );
}

void JogCmdArrayWriteReal64::execute( JogVM* vm )
{
  double value = vm->pop_double();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((double*)array->data)[index] = value;
  vm->push( value );
}

void JogCmdArrayWriteReal32::execute( JogVM* vm )
{
  double value = vm->pop_double();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((float*)array->data)[index] = (float) value;
  vm->push( value );
}

void JogCmdArrayWriteInt64::execute( JogVM* vm )
{
  JogInt64 value = vm->pop_data();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((JogInt64*)array->data)[index] = value;
  vm->push( value );
}

void JogCmdArrayWriteInt32::execute( JogVM* vm )
{
  JogInt64 value = vm->pop_data();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((JogInt32*)array->data)[index] = (JogInt32) value;
  vm->push( value );
}

void JogCmdArrayWriteInt16::execute( JogVM* vm )
{
  JogInt64 value = vm->pop_data();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((JogInt16*)array->data)[index] = (JogInt16) value;
  vm->push( value );
}

void JogCmdArrayWriteInt8::execute( JogVM* vm )
{
  JogInt64 value = vm->pop_data();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((JogInt8*)array->data)[index] = (JogInt8) value;
  vm->push( value );
}

void JogCmdArrayWriteChar::execute( JogVM* vm )
{
  JogInt64 value = vm->pop_data();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((JogChar*)array->data)[index] = (JogChar) value;
  vm->push( value );
}

void JogCmdArrayWriteBoolean::execute( JogVM* vm )
{
  JogInt64 value = vm->pop_data();
  int index = vm->pop_int();
  JogRef obj = vm->pop_ref();
  JogObject* array = obj.null_check(t);
  array->index_check(t,index);
  ((char*)array->data)[index] = (char) value;
  vm->push( value );
}

