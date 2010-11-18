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
//     - Strings (basics, string +/+= string)
//
//   TODO
//     - Arrays (multidimensional, +=, etc.)
//     - String +/+= primitive types
//     - Autoboxing
//     - Generics
//     - Respect private/protected access qualifiers
//     - Maybe inner classes, maybe exceptions

class Test
{
  Test()
  {
    String st = "World";
    Object obj = "!";
    st = st + obj;
    st = "Hello " + st;
    println(st);

    Number n = new Integer(5);
    println(n.intValue());

    /*
    double start_ms = System.currentTimeMillis();
    for (int i=1; i<=1000000; ++i)
    {
    }
    //double end_ms = System.currentTimeMillis();
    //println( start_ms );
    //println( end_ms );
    //println( (end_ms - start_ms) );

    // This might crash on Mac - will investigate.
    print( "Looped a million times in " );
    print( (System.currentTimeMillis() - start_ms) / 1000.0 );
    print( " seconds." );
    */
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
abstract class Number
{
  abstract byte   byteValue();
  abstract double doubleValue();
  abstract float  floatValue();
  abstract int    intValue();
  abstract long   longValue();
  abstract short  shortValue();
}

class Integer extends Number
{
  int value;

  public Integer( int value )
  {
    this.value = value;
  }

  public int intValue()
  {
    return value;
  }
}

class Object
{
  public Object() { }

  public String toString() { return "(An Object)"; }

  static public void print( boolean n ) { System.out.print(n); }
  static public void print( char ch ) { System.out.print(ch); }
  static public void print( double n ) { System.out.print(n); }
  static public void print( int n ) { System.out.print(n); }
  static public void print( long n ) { System.out.print(n); }
  static public void print( Object obj ) { System.out.print(obj.toString()); }
  static public void print( String st ) { System.out.print(st); }
  static public void println( boolean n ) { System.out.println(n); }
  static public void println( char ch ) { System.out.println(ch); }
  static public void println( double n ) { System.out.println(n); }
  static public void println( int n ) { System.out.println(n); }
  static public void println( long n ) { System.out.println(n); }
  static public void println( Object obj ) { System.out.println(obj.toString()); }
  static public void println( String st ) { System.out.println(st); }
}

class String
{
  // Note: the native layer assumes these two properties are defined as they are.
  char[] data;
  int    hash_code;

  public String( char[] data )
  {
    int count = data.length;
    this.data = new char[count];
    for (int i=0; i<count; ++i)
    {
      char ch = data[i];
      hash_code = (hash_code << 1) + ch;
      this.data[i] = ch;
    }
  }

  public String toString()
  {
    return this;
  }

  public int length()
  {
    return data.length;
  }

  public String concat( String other )
  {
    char[] new_data = new char[data.length + other.data.length];

    int c = data.length;
    for (int i=0; i<c; ++i) new_data[i] = data[i];

    int c2 = other.data.length;
    for (int i=0; i<c2; ++i) new_data[c++] = other.data[i];

    return new String(new_data);
  }
}

class System
{
  static PrintWriter out;

  static
  {
    out = new PrintWriter();
  }

  native static long currentTimeMillis();
}

class PrintWriter
{
  native void print( boolean n );
  native void print( char ch );
  native void print( double n );
  native void print( int n );
  native void print( long n );
  native void print( String st );

  void println( boolean n ) { print(n); print('\n'); }
  void println( char ch ) { print(ch); print('\n'); }
  void println( double n ) { print(n); print('\n'); }
  void println( int n ) { print(n); print('\n'); }
  void println( long n ) { print(n); print('\n'); }
  void println( String st ) { print(st); print('\n'); }
}

