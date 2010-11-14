#include "jog.h"

JogParser::JogParser( const char* filename ) : this_method(NULL)
{
  scanner = new JogScanner( new JogReader(filename) );
}

JogTypeInfo* JogParser::parse_type_def()
{
  int quals = parse_type_qualifiers();
  Ref<JogToken> t = scanner->peek();

  if (scanner->consume(TOKEN_CLASS))
  {
    return parse_type_def( t, quals|JOG_QUALIFIER_CLASS, "Class name expected." );
  }
  else
  {
    if (quals)
    {
      throw t->error( "Keyword 'class' expected." );
    }
  }

  return NULL;
}

JogTypeInfo* JogParser::parse_type_def( Ref<JogToken> t, int quals, const char* missing_name_mesg )
{
  if ((quals & (JOG_QUALIFIER_PUBLIC|JOG_QUALIFIER_PROTECTED|JOG_QUALIFIER_PRIVATE)) == 0)
  {
    // protected is the default
    quals |= JOG_QUALIFIER_PROTECTED;
  }

  Ref<JogString> name = scanner->must_read_id( missing_name_mesg );

  JogTypeInfo* type = JogTypeInfo::create( t, quals, name );

  if (scanner->consume(TOKEN_EXTENDS))
  {
    type->base_class = parse_data_type();
  }

  // make one empty static initializer for setting up initial class property values
  type->static_initializers.add(
    new JogMethodInfo( t, JOG_QUALIFIER_STATIC, type, NULL, new JogString("static") ) 
    );

  scanner->must_consume(TOKEN_LCURLY,"Opening '{' expected.");

  while (parse_member(type))
  {
  }

  scanner->must_consume(TOKEN_RCURLY,"Closing '}' expected.");

  return type;
}

int JogParser::parse_type_qualifiers()
{
  int quals = 0;
  
  for (;;)
  {
    Ref<JogToken> t = scanner->peek();

    // public
    if (scanner->consume(TOKEN_PUBLIC))
    {
      quals |= JOG_QUALIFIER_PUBLIC;
      if (quals & (JOG_QUALIFIER_PROTECTED | JOG_QUALIFIER_PRIVATE))
      {
        throw t->error( "'public' cannot be mixed with 'protected' or 'private'." );
      }
      continue;
    }

    // protected
    if (scanner->consume(TOKEN_PROTECTED))
    {
      quals |= JOG_QUALIFIER_PROTECTED;
      if (quals & (JOG_QUALIFIER_PUBLIC | JOG_QUALIFIER_PRIVATE))
      {
        throw t->error( "'protected' cannot be mixed with 'public' or 'private'." );
      }
      continue;
    }

    // private
    if (scanner->consume(TOKEN_PRIVATE))
    {
      quals |= JOG_QUALIFIER_PRIVATE;
      if (quals & (JOG_QUALIFIER_PUBLIC | JOG_QUALIFIER_PROTECTED))
      {
        throw t->error( "'private' cannot be mixed with 'public' or 'protected'." );
      }
      continue;
    }

    return quals;
  }
}

int JogParser::parse_member_qualifiers()
{
  int quals = 0;
  
  for (;;)
  {
    Ref<JogToken> t = scanner->peek();

    // public
    if (scanner->consume(TOKEN_PUBLIC))
    {
      quals |= JOG_QUALIFIER_PUBLIC;
      if (quals & (JOG_QUALIFIER_PROTECTED | JOG_QUALIFIER_PRIVATE))
      {
        throw t->error( "'public' cannot be mixed with 'protected' or 'private'." );
      }
      continue;
    }

    // protected
    if (scanner->consume(TOKEN_PROTECTED))
    {
      quals |= JOG_QUALIFIER_PROTECTED;
      if (quals & (JOG_QUALIFIER_PUBLIC | JOG_QUALIFIER_PRIVATE))
      {
        throw t->error( "'protected' cannot be mixed with 'public' or 'private'." );
      }
      continue;
    }

    // private
    if (scanner->consume(TOKEN_PRIVATE))
    {
      quals |= JOG_QUALIFIER_PRIVATE;
      if (quals & (JOG_QUALIFIER_PUBLIC | JOG_QUALIFIER_PROTECTED))
      {
        throw t->error( "'private' cannot be mixed with 'public' or 'protected'." );
      }
      continue;
    }

    // static
    if (scanner->consume(TOKEN_STATIC))
    {
      quals |= JOG_QUALIFIER_STATIC;
      continue;
    }

    // static
    if (scanner->consume(TOKEN_NATIVE))
    {
      quals |= JOG_QUALIFIER_NATIVE;
      continue;
    }

    return quals;
  }
}

bool JogParser::parse_member( JogTypeInfo* type )
{
  if (scanner->next_is(TOKEN_RCURLY)) return false;

  Ref<JogToken> t = scanner->peek();
  int quals = parse_member_qualifiers();

  Ref<JogToken>    t2 = scanner->peek();
  if (quals == JOG_QUALIFIER_STATIC && scanner->next_is(TOKEN_LCURLY))
  {
    // static initializer block
    Ref<JogMethodInfo> m = new JogMethodInfo( t, quals, type, NULL, new JogString("static") );
    this_method = *m;
    type->static_initializers.add(*m);

    scanner->read();  // open curly
    while ( !scanner->next_is(TOKEN_RCURLY) )
    {
      Ref<JogCmd> statement = parse_statement();
      if (*statement)
      {
        m->statements->add(statement);
      }
    }
    scanner->must_consume( TOKEN_RCURLY, "Closing '}' expected." );
    return true;
  }

  JogTypeInfo*     data_type = parse_data_type();

  if (scanner->next_is(TOKEN_LPAREN))
  {
    if (data_type->name->equals(type->name))
    {
      // Constructor
      if (quals & JOG_QUALIFIER_STATIC)
      {
        throw t->error( "Constructors cannot be 'static'." );
      }
      quals |= JOG_QUALIFIER_CONSTRUCTOR;
      Ref<JogMethodInfo> m = new JogMethodInfo( t, quals, type, NULL, new JogString("<init>") );
      this_method = *m;
      parse_params(m);
      type->methods.add(*m);

      scanner->must_consume( TOKEN_LCURLY, "Opening '{' expected." );
      while ( !scanner->next_is(TOKEN_RCURLY) )
      {
        Ref<JogCmd> statement = parse_statement();
        if (*statement)
        {
          m->statements->add(statement);
        }
      }
      scanner->must_consume( TOKEN_RCURLY, "Closing '}' expected." );
      return true;
    }
    else
    {
      throw t2->error( "Method is missing return type declaration." );
    }
  }

  Ref<JogToken>    name_t = scanner->peek();
  Ref<JogString>   name = scanner->must_read_id( "Name expected after data type." );

  if (scanner->next_is(TOKEN_LPAREN))
  {
    // Method

    // Malformed constructors
    if (name->equals(type->name))
    {
      throw t2->error( "Constructors should not specify a return type." );
    }

  
    // Other methods
    Ref<JogMethodInfo> m = new JogMethodInfo( t, quals, type, data_type, name );
    this_method = *m;
    parse_params(m);
    if (m->is_static())
    {
      type->class_methods.add(*m);
    }
    else
    {
      type->methods.add(*m);
    }

    if (quals & JOG_QUALIFIER_NATIVE)
    {
      scanner->must_consume(TOKEN_SEMICOLON,"Expected ';' at end of native method declaration." );
    }
    else if ( !scanner->consume(TOKEN_SEMICOLON) )
    {
      scanner->must_consume( TOKEN_LCURLY, "Opening '{' expected." );
      while ( !scanner->next_is(TOKEN_RCURLY) )
      {
        Ref<JogCmd> statement = parse_statement();
        if (*statement)
        {
          m->statements->add(statement);
        }
      }
      scanner->must_consume( TOKEN_RCURLY, "Closing '}' expected." );
    }

  }
  else
  {
    if (data_type == NULL)
    {
      throw t->error( "'void' cannot be used as a property type." );
    }

    if (quals & JOG_QUALIFIER_NATIVE)
    {
      throw t->error( "The 'native' qualifier cannot be used for properties." );
    }

    // property
    bool first = true;
    do
    {
      if (first) 
      {
        first = false;
      }
      else 
      {
        name_t = scanner->peek();
        name = scanner->must_read_id( "Property name expected." );
      }

      Ref<JogPropertyInfo> p = new JogPropertyInfo( name_t, quals, type, data_type, name, parse_initial_value() );
      if (p->is_static())
      {
        type->class_properties.add(*p);
      }
      else
      {
        type->properties.add(*p);
      }

    }
    while (scanner->consume(TOKEN_COMMA));

    scanner->must_consume_semicolon(t);
  }
  return true;
}

void JogParser::parse_params( Ref<JogMethodInfo> m )
{
  scanner->must_consume(TOKEN_LPAREN,"Opening '(' expected.");

  if ( !scanner->consume(TOKEN_RPAREN) )
  {
    do
    {
      Ref<JogToken>    t = scanner->peek();
      JogTypeInfo*     type = parse_data_type();
      if (type == NULL)
      {
        throw t->error( "'void' cannot be used as a parameter type." );
      }

      Ref<JogString>   name = scanner->must_read_id( "Name expected after data type." );
      m->parameters.add( new JogLocalVarInfo(t,type,name) );
    }
    while (scanner->consume(TOKEN_COMMA));
    scanner->must_consume( TOKEN_RPAREN, "Closing ')' expected." );
  }
}

JogTypeInfo* JogParser::parse_data_type()
{
  Ref<JogToken> t = scanner->peek();
  Ref<JogString> name = scanner->must_read_id("Data type expected (void, int, String, ...).");
  while (scanner->consume(TOKEN_PERIOD))
  {
    name->add( "." );
    name->add( scanner->must_read_id("Sub-package or class name expected.") );
  }
  while (scanner->consume(TOKEN_LBRACKET))
  {
    name->add( "[" );
    scanner->must_consume( TOKEN_RBRACKET, "Closing ']' expected." );
  }
  return JogTypeInfo::reference( t, name );
}

Ref<JogCmd> JogParser::parse_initial_value()
{
  if ( !scanner->consume(TOKEN_ASSIGN) ) return NULL;

  return parse_expression();
}

Ref<JogCmd> JogParser::parse_statement( bool require_semicolon )
{
  if (require_semicolon && scanner->consume(TOKEN_SEMICOLON)) return NULL;
  else if (scanner->next_is(TOKEN_RPAREN)) return NULL;

  Ref<JogToken> t = scanner->peek();

  if (scanner->consume(TOKEN_LCURLY))
  {
    Ref<JogCmdBlock> block = new JogCmdBlock(t);
    while ( !scanner->consume(TOKEN_RCURLY) )
    {
      block->statements->add( parse_statement() );
    }
    return *block;
  }

  if (scanner->consume(TOKEN_RETURN))
  {
    Ref<JogCmd> cmd = new JogCmdReturnValue( t, parse_expression() );
    if (require_semicolon) scanner->must_consume_semicolon(t);
    return *cmd;
  }

  if (scanner->consume(TOKEN_IF))
  {
    scanner->must_consume( TOKEN_LPAREN, "Opening '(' expected." );
    Ref<JogCmdIf> conditional = new JogCmdIf( t, parse_expression() );
    scanner->must_consume( TOKEN_RPAREN, "Closing ')' expected." );

    if (scanner->next_is(TOKEN_SEMICOLON))
    {
      throw scanner->peek()->error( "Semicolon not allowed here; use \"{}\" to denote an empty body." );
    }

    conditional->body = parse_statement();
    if (scanner->consume(TOKEN_ELSE))
    {
      if (scanner->next_is(TOKEN_SEMICOLON))
      {
        throw scanner->peek()->error( "Semicolon not allowed here; use \"{}\" to denote an empty body." );
      }
      conditional->else_body = parse_statement();
    }
    return *conditional;
  }

  if (scanner->consume(TOKEN_WHILE))
  {
    scanner->must_consume( TOKEN_LPAREN, "Opening '(' expected." );
    Ref<JogCmdControlStructure> loop = new JogCmdWhile( t, parse_expression() );
    scanner->must_consume( TOKEN_RPAREN, "Closing ')' expected." );
    if (scanner->next_is(TOKEN_SEMICOLON))
    {
      throw scanner->peek()->error( "Semicolon not allowed here; use \"{}\" to denote an empty body." );
    }
    loop->body = parse_statement();
    return *loop;
  }

  if (scanner->consume(TOKEN_FOR))
  {
    scanner->must_consume( TOKEN_LPAREN, "Opening '(' expected." );
    Ref<JogCmd> init_expr = parse_statement();
    Ref<JogCmd> condition = parse_expression();
    scanner->must_consume( TOKEN_SEMICOLON, "';' expected." );
    Ref<JogCmd> var_mod = parse_statement(false);
    scanner->must_consume( TOKEN_RPAREN, "Closing ')' expected." );
    Ref<JogCmdFor> loop = new JogCmdFor( t, init_expr, condition, var_mod );
    if (scanner->next_is(TOKEN_SEMICOLON))
    {
      throw scanner->peek()->error( "Semicolon not allowed here; use \"{}\" to denote an empty body." );
    }
    loop->body = parse_statement();
    return *loop;
  }

  Ref<JogCmd> expr = parse_expression();
  if (scanner->next_is(TOKEN_ID))
  {
    // local variable declaration
    JogTypeInfo* var_type = expr->reinterpret_as_type();
    if ( !var_type )
    {
      throw expr->error( "Datatype expected." );
    }

    Ref<JogCmdLocalVarDeclarations> locals = new JogCmdLocalVarDeclarations(expr->t);
    do
    {
      Ref<JogToken>    t2 = scanner->peek();
      Ref<JogString>   name = scanner->must_read_id( "Identifier expected." );
      Ref<JogCmdLocalVarDeclaration> decl = new JogCmdLocalVarDeclaration(t2,var_type,name);
      if (scanner->consume(TOKEN_ASSIGN))
      {
        decl->initial_value = parse_expression();
      }
      locals->add(*decl);
    }
    while (scanner->consume(TOKEN_COMMA));

    if (require_semicolon) scanner->must_consume_semicolon(t);
    return *locals;
  }

  if (require_semicolon) scanner->must_consume_semicolon(t);

  return *(expr->discarding_result());
}

Ref<JogCmd> JogParser::parse_expression()
{
  Ref<JogCmd> cmd = parse_assignment();
  return cmd;
  //return parse_assignment();
}

Ref<JogCmd> JogParser::parse_assignment()
{
  Ref<JogCmd> expr = parse_conditional();
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_ASSIGN))
  {
    expr = new JogCmdAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_ADD_ASSIGN))
  {
    expr = new JogCmdAddAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_SUB_ASSIGN))
  {
    expr = new JogCmdSubAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_MUL_ASSIGN))
  {
    expr = new JogCmdMulAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_DIV_ASSIGN))
  {
    expr = new JogCmdDivAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_MOD_ASSIGN))
  {
    expr = new JogCmdModAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_AND_ASSIGN))
  {
    expr = new JogCmdAndAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_OR_ASSIGN))
  {
    expr = new JogCmdOrAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_XOR_ASSIGN))
  {
    expr = new JogCmdXorAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_SHL_ASSIGN))
  {
    expr = new JogCmdLeftShiftAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_SHRX_ASSIGN))
  {
    expr = new JogCmdRightXShiftAssign(t,expr,parse_assignment());
  }
  else if (scanner->consume(TOKEN_SHR_ASSIGN))
  {
    expr = new JogCmdRightShiftAssign(t,expr,parse_assignment());
  }

  return expr;
}

Ref<JogCmd> JogParser::parse_conditional()
{
  Ref<JogCmd> expr = parse_logical_or();
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_QUESTIONMARK))
  {
    Ref<JogCmd> true_value = parse_conditional();
    scanner->must_consume( TOKEN_COLON, "':' expected." );
    Ref<JogCmd> false_value = parse_conditional();
    return new JogCmdConditional(t,expr,true_value,false_value);
  }
  else
  {
    return expr;
  }
}

Ref<JogCmd> JogParser::parse_logical_or()
{
  return parse_logical_or( parse_logical_and() );
}

Ref<JogCmd> JogParser::parse_logical_or( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_LOGICAL_OR))
  {
    return parse_logical_or( new JogCmdLogicalOr(t,lhs,parse_logical_and()) );
  }
  else
  {
    return lhs;
  }
}

// &&
Ref<JogCmd> JogParser::parse_logical_and()
{
  return parse_logical_and( parse_bitwise_or() );
}

Ref<JogCmd> JogParser::parse_logical_and( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_LOGICAL_AND))
  {
    return parse_logical_and( new JogCmdLogicalAnd(t,lhs,parse_bitwise_or()) );
  }
  else
  {
    return lhs;
  }
}

// |
Ref<JogCmd> JogParser::parse_bitwise_or()
{
  return parse_bitwise_or( parse_bitwise_xor() );
}

Ref<JogCmd> JogParser::parse_bitwise_or( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_PIPE))
  {
    return parse_bitwise_or( new JogCmdBitwiseOr(t,lhs,parse_bitwise_xor()) );
  }
  else
  {
    return lhs;
  }
}

// ^
Ref<JogCmd> JogParser::parse_bitwise_xor()
{
  return parse_bitwise_xor( parse_bitwise_and() );
}

Ref<JogCmd> JogParser::parse_bitwise_xor( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_CARET))
  {
    return parse_bitwise_xor( new JogCmdBitwiseXor(t,lhs,parse_bitwise_and()) );
  }
  else
  {
    return lhs;
  }
}

// &
Ref<JogCmd> JogParser::parse_bitwise_and()
{
  return parse_bitwise_and( parse_equality() );
}

Ref<JogCmd> JogParser::parse_bitwise_and( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_AMPERSAND))
  {
    return parse_bitwise_and( new JogCmdBitwiseAnd(t,lhs,parse_equality()) );
  }
  else
  {
    return lhs;
  }
}

// ==, !=
Ref<JogCmd> JogParser::parse_equality()
{
  return parse_equality( parse_relational() );
}

Ref<JogCmd> JogParser::parse_equality( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_EQ))
  {
    return parse_equality( new JogCmdEQ(t,lhs,parse_relational()) );
  }
  else if (scanner->consume(TOKEN_NE))
  {
    return parse_equality( new JogCmdNE(t,lhs,parse_relational()) );
  }
  else
  {
    return lhs;
  }
}

// <, <=, >, >=, instanceof
Ref<JogCmd> JogParser::parse_relational()
{
  return parse_relational( parse_shift() );
}

Ref<JogCmd> JogParser::parse_relational( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_LT))
  {
    return parse_relational( new JogCmdLT(t,lhs,parse_shift()) );
  }
  else if (scanner->consume(TOKEN_LE))
  {
    return parse_relational( new JogCmdLE(t,lhs,parse_shift()) );
  }
  else if (scanner->consume(TOKEN_GT))
  {
    return parse_relational( new JogCmdGT(t,lhs,parse_shift()) );
  }
  else if (scanner->consume(TOKEN_GE))
  {
    return parse_relational( new JogCmdGE(t,lhs,parse_shift()) );
  }
  else if (scanner->consume(TOKEN_INSTANCEOF))
  {
    return parse_relational( new JogCmdInstanceOf(t,lhs,parse_data_type()) );
  }
  else
  {
    return lhs;
  }
}

// <<, >>, >>>
Ref<JogCmd> JogParser::parse_shift()
{
  return parse_shift( parse_translate() );
}

Ref<JogCmd> JogParser::parse_shift( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_SHL))
  {
    return parse_shift( new JogCmdLeftShift(t,lhs,parse_translate()) );
  }
  else if (scanner->consume(TOKEN_SHR))
  {
    return parse_shift( new JogCmdRightShift(t,lhs,parse_translate()) );
  }
  else if (scanner->consume(TOKEN_SHRX))
  {
    return parse_shift( new JogCmdRightXShift(t,lhs,parse_translate()) );
  }
  else
  {
    return lhs;
  }
}

// +, -
Ref<JogCmd> JogParser::parse_translate()
{
  return parse_translate( parse_scale() );
}

Ref<JogCmd> JogParser::parse_translate( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_PLUS))
  {
    return parse_translate( new JogCmdAdd(t,lhs,parse_scale()) );
  }
  else if (scanner->consume(TOKEN_MINUS))
  {
    return parse_translate( new JogCmdSub(t,lhs,parse_scale()) );
  }
  else
  {
    return lhs;
  }
}

// *, /, %
Ref<JogCmd> JogParser::parse_scale()
{
  return parse_scale( parse_prefix_unary() );
}

Ref<JogCmd> JogParser::parse_scale( Ref<JogCmd> lhs )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_STAR))
  {
    return parse_scale( new JogCmdMul(t,lhs,parse_prefix_unary()) );
  }
  else if (scanner->consume(TOKEN_SLASH))
  {
    return parse_scale( new JogCmdDiv(t,lhs,parse_prefix_unary()) );
  }
  else if (scanner->consume(TOKEN_PERCENT))
  {
    return parse_scale( new JogCmdMod(t,lhs,parse_prefix_unary()) );
  }
  else
  {
    return lhs;
  }
}

// Prefix Unary - (cast), new, ++, --, +, -, !, ~
Ref<JogCmd> JogParser::parse_prefix_unary()
{
  Ref<JogToken> t = scanner->peek();

  if (scanner->next_is(TOKEN_LPAREN))
  {
    // MIGHT have a cast
    scanner->set_mark();

    scanner->read();
    if (scanner->next_is(TOKEN_ID))
    {
      if (scanner->next_is("boolean")
        || scanner->next_is("double")
        || scanner->next_is("float")
        || scanner->next_is("long")
        || scanner->next_is("int")
        || scanner->next_is("short")
        || scanner->next_is("char")
        || scanner->next_is("byte"))
      {
        scanner->clear_mark();
        JogTypeInfo* to_type = parse_data_type();
        scanner->must_consume(TOKEN_RPAREN,"Closing ')' expected.");
        return new JogCmdCast( t, parse_prefix_unary(), to_type );
      }
    }

    scanner->rewind_to_mark();
  }

  if (scanner->consume(TOKEN_NEW))
  {
    JogTypeInfo* of_type = parse_data_type();
    Ref<JogCmdList>  args = parse_args(true);
    return new JogCmdNewObject( t, of_type, args );
  }
  else if (scanner->consume(TOKEN_INCREMENT))
  {
    return new JogCmdPreIncrement( t, parse_prefix_unary() );
  }
  else if (scanner->consume(TOKEN_DECREMENT))
  {
    return new JogCmdPreDecrement( t, parse_prefix_unary() );
  }
  else if (scanner->consume(TOKEN_PLUS))
  {
    // discard "+a" and just keep "a".
    return parse_prefix_unary();
  }
  else if (scanner->consume(TOKEN_MINUS))
  {
    return new JogCmdNegate( t, parse_prefix_unary() );
  }
  else if (scanner->consume(TOKEN_BANG))
  {
    return new JogCmdLogicalNot( t, parse_prefix_unary() );
  }
  else if (scanner->consume(TOKEN_TILDE))
  {
    return new JogCmdBitwiseNot( t, parse_prefix_unary() );
  }
  else
  {
    return parse_postfix_unary();
  }
}

// ++, --, ., (), []
Ref<JogCmd> JogParser::parse_postfix_unary()
{
  return parse_postfix_unary( parse_term() );
}

Ref<JogCmd> JogParser::parse_postfix_unary( Ref<JogCmd> operand )
{
  Ref<JogToken> t = scanner->peek();
  if (scanner->consume(TOKEN_INCREMENT))
  {
    return parse_postfix_unary( new JogCmdPostIncrement(t,operand) );
  }
  else if (scanner->consume(TOKEN_DECREMENT))
  {
    return parse_postfix_unary( new JogCmdPostDecrement(t,operand) );
  }
  else if (scanner->consume(TOKEN_PERIOD))
  {
    Ref<JogCmd> cmd = parse_postfix_unary( new JogCmdMemberAccess(t,operand,parse_term()) );
    return cmd;
  }
  else if (scanner->consume(TOKEN_LBRACKET))
  {
    Ref<JogCmd> index = parse_expression();
    scanner->must_consume(TOKEN_RBRACKET,"Closing ']' expected.");
    return parse_postfix_unary( new JogCmdArrayAccess(t,operand,index) );
  }
  else
  {
    return operand;
  }
}

Ref<JogCmd> JogParser::parse_term()
{
  switch (scanner->peek()->type)
  {
    case TOKEN_LITERAL_DOUBLE:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralReal64(t,t->content);
      }

    case TOKEN_LITERAL_FLOAT:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralReal32(t,t->content);
      }

    case TOKEN_LITERAL_LONG:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralInt64(t,t->content);
      }

    case TOKEN_LITERAL_INT:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralInt32(t,t->content);
      }

    case TOKEN_LITERAL_CHAR:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralChar(t,t->content->data[0]);
      }

    case TOKEN_STRING:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralString(t,t->content);
      }

    case TOKEN_FALSE:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralBoolean(t,false);
      }

    case TOKEN_TRUE:
      {
        Ref<JogToken> t = scanner->read();
        return new JogCmdLiteralBoolean(t,true);
      }

    case TOKEN_LPAREN:
      {
        scanner->read();
        Ref<JogCmd> expr = parse_expression();
        scanner->must_consume( TOKEN_RPAREN, "Closing ')' expected." );
        return expr;
      }

    case TOKEN_SUPER:
      if (scanner->peek(2)->type == TOKEN_LPAREN)
      {
        Ref<JogToken> t = scanner->read();
        if ( !this_method->is_constructor() )
        {
          throw t->error( "Use \"super.methodname\" to call a superclass method." );
        }
        if (this_method->statements->count())
        {
          throw t->error( "The call to the superclass constructor must be the first statement." );
        }
        Ref<JogCmdList> args = parse_args(true);
        this_method->calls_super_constructor = true;
        return new JogCmdCallSuperConstructor(t,args);
      }
      else
      {
        Ref<JogToken>    t = scanner->read();
        scanner->must_consume(TOKEN_PERIOD,"'.' expected.");
        Ref<JogString>   name = scanner->must_read_id( "Method name expected." );
        Ref<JogCmdList>  args = parse_args(true);
        return new JogCmdSuperCall( t, name, args );
      }

    case TOKEN_ID:
      return parse_construct();

    case TOKEN_NULL:
      return new JogCmdNullRef(scanner->read());
  }


  // Check out of place keywords
  const char* mesg;
  if (scanner->next_is(TOKEN_ELSE))
  {
    mesg = "Unexpected 'else' does not match any 'if'.";
  }
  else
  {
    mesg = "Expression expected.";
  }
  throw scanner->read()->error( mesg );
}

Ref<JogCmd> JogParser::parse_construct()
{
  Ref<JogToken>    t = scanner->peek();

  Ref<JogString>   name = scanner->must_read_id( "Identifier expected." );

  if (scanner->next_is(TOKEN_LT))
  {
    // Might have a data type, e.g. "ArrayList<Double>".  Try parsing that.
    // TODO
  }

  Ref<JogCmdList>  args = parse_args(false);
  if (*args == NULL) return new JogCmdIdentifier(t,name);

  return new JogCmdMethodCall( t, name, args );
}

Ref<JogCmdList> JogParser::parse_args( bool required )
{
  Ref<JogToken> t = scanner->peek();

  if ( !scanner->consume(TOKEN_LPAREN) )
  {
    if (required) throw t->error( "Opening '(' expected." );
    return NULL;
  }

  Ref<JogCmdList> args = new JogCmdArgs(t);
  if ( !scanner->consume(TOKEN_RPAREN) )
  {
    do
    {
      args->add( parse_expression() );
    }
    while (scanner->consume(TOKEN_COMMA));
    scanner->must_consume( TOKEN_RPAREN, "Closing ')' expected." );
  }

  return args;
}

