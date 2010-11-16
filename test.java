// Project state:
//   WORKING
//     - Classes, extended classes, polymorphism
//     - this() and super() constructor calls, super.method() calls
//     - Class methods, object methods, overloads, overrides
//     - Class and object property initial value assignments, static initialization blocks
//     - Primitive types and operations
//     - Object types
//     - System.out.print/println for primitive types
//     - Arrays (length, get, set)
//
//   TODO
//     - Arrays (+=, etc.)
//     - Strings
//     - Generics
//     - Respect private/protected access qualifiers
//     - Maybe inner classes, maybe exceptions

class Test
{

  Test()
  {

    char[] chars = new char[12];

    chars[0]  = 'H';
    chars[1]  = 'e';
    chars[2]  = 'l';
    chars[3]  = 'l';
    chars[4]  = 'o';
    chars[5]  = ' ';
    chars[6]  = 'W';
    chars[7]  = 'o';
    chars[8]  = 'r';
    chars[9]  = 'l';
    chars[10] = 'd';
    chars[11] = '!';

    println( chars.length );
    for (int i=0; i<chars.length; ++i) { print( chars[i] ); }
    print('\n');
  }
}

class Alpha
{
  int number;

  Alpha( int n ) { number = n; }

  public int toInt() { return number; }
}

class Beta extends Alpha
{
  public Beta() { super(1); }
  public int toInt() { return super.toInt() + 2; }
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

