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
  static int a = 4;
  static int b = a + 1;
  int n = 2;

  Test()
  {
    println( (new Alpha()).number );
    println(n);
    println(a);
    println(b);
  }

  static public void test()
  {
    println(true);
  }
}

class Alpha
{
  int number = 3;

  public Alpha() { Test.test(); }

  public Alpha( int number )
  {
    this.number = number;
  }

  public int five() { return 5; }

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

  static public void print( double n ) { System.out.print(n); }
  static public void print( int n ) { System.out.print(n); }
  static public void print( boolean n ) { System.out.print(n); }
  static public void print( char ch ) { System.out.print(ch); }
  static public void println( double n ) { System.out.println(n); }
  static public void println( int n ) { System.out.println(n); }
  static public void println( boolean n ) { System.out.println(n); }
  static public void println( char ch ) { System.out.println(ch); }
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

