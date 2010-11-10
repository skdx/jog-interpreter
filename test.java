// Project state:
//   WORKING
//     - Classes, extended classes, polymorphism, and super() constructor calls
//     - Class methods, object methods, overloads, overrides
//     - Static initialization blocks
//     - Primitive types and operations
//     - Object types
//     - System.out.print/println for primitive types
//
//   TODO
//     - Initial property value assignments
//     - this() constructor calls
//     - super.method() calls
//     - Arrays
//     - Strings
//     - Generics
//     - Respect private/protected access qualifiers
//     - Maybe inner classes

class Test
{
  int n;

  Test()
  {
    Alpha a = new Beta(5);
    System.out.println( a.nextValue() );
    System.out.println( a.nextValue() );
    System.out.println( a.number );
  }
}

class Alpha
{
  int number;

  public Alpha( int number )
  {
    this.number = number;
  }

  public int nextValue()
  {
    return 0;
  }
}

class Beta extends Alpha
{
  public Beta( int n )
  {
    super(n*2);
  }

  public int nextValue()
  {
    return number++;
  }
}

//==============================================================================
// STANDARD LIBRARY - DON'T DELETE
//==============================================================================
class Object
{
  public Object() { }
}

class String
{
}

class System
{
  static PrintWriter out;

  static
  {
    out = new PrintWriter();
  }
}

class PrintWriter
{
  native void print( double n );
  native void print( int n );
  native void print( boolean n );
  native void print( char ch );

  void println( double n ) { print(n); print('\n'); }
  void println( int n ) { print(n); print('\n'); }
  void println( boolean n ) { print(n); print('\n'); }
  void println( char ch ) { print(ch); print('\n'); }
}

