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
    println( (new StringBuilder(2)).append("what").append("up") );

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
    this( data, data.length );
  }

  public String( char[] data, int count )
  {
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

class StringBuilder
{
  char[] data;
  int    size;

  public StringBuilder()
  {
    data = new char[16];
  }

  public StringBuilder( int capacity )
  {
    data = new char[capacity];
  }

  public StringBuilder( String initial_contents )
  {
    this( initial_contents.length() );

    size = initial_contents.length();
    for (int i=0; i<size; ++i) data[i] = initial_contents.data[i];
  }

  public String toString()
  {
    return new String(data,size);
  }

  public void ensureCapacity( int min_capacity )
  {
    if (data.length >= min_capacity) return;

    int new_cap = data.length * 2 + 2;
    if (min_capacity > new_cap) new_cap = min_capacity;

    char[] new_data = new char[new_cap];
    for (int i=0; i<size; ++i) new_data[i] = data[i];

    data = new_data;
  }

  public StringBuilder append( String st )
  {
    int count = st.length();
    ensureCapacity( size + count );

    for (int i=0; i<count; ++i)
    {
      data[size++] = st.data[i];
    }
    return this;
  }

  public StringBuilder append( char ch )
  {
    ensureCapacity( size + 1 );
    data[size++] = ch;
    return this;
  }
}

abstract class Number
{
  abstract byte   byteValue();
  abstract double doubleValue();
  abstract float  floatValue();
  abstract int    intValue();
  abstract long   longValue();
  abstract short  shortValue();
}

class Byte extends Number
{
  byte value;

  public Byte( byte value )
  {
    this.value = value;
  }

  byte   byteValue() { return (byte) value; }
  double doubleValue() { return (double) value; }
  float  floatValue() { return (float) value; }
  int    intValue() { return (int) value; }
  long   longValue() { return (long) value; }
  short  shortValue() { return (short) value; }
}

class Double extends Number
{
  double value;

  public Double( double value )
  {
    this.value = value;
  }

  byte   byteValue() { return (byte) value; }
  double doubleValue() { return (double) value; }
  float  floatValue() { return (float) value; }
  int    intValue() { return (int) value; }
  long   longValue() { return (long) value; }
  short  shortValue() { return (short) value; }
}

class Float extends Number
{
  float value;

  public Float( float value )
  {
    this.value = value;
  }

  byte   byteValue() { return (byte) value; }
  double doubleValue() { return (double) value; }
  float  floatValue() { return (float) value; }
  int    intValue() { return (int) value; }
  long   longValue() { return (long) value; }
  short  shortValue() { return (short) value; }
}

class Integer extends Number
{
  int value;

  public Integer( int value )
  {
    this.value = value;
  }

  byte   byteValue() { return (byte) value; }
  double doubleValue() { return (double) value; }
  float  floatValue() { return (float) value; }
  int    intValue() { return (int) value; }
  long   longValue() { return (long) value; }
  short  shortValue() { return (short) value; }

  //public String toString()
  //{
  //}
}

class Long extends Number
{
  long value;

  public Long( long value )
  {
    this.value = value;
  }

  byte   byteValue() { return (byte) value; }
  double doubleValue() { return (double) value; }
  float  floatValue() { return (float) value; }
  int    intValue() { return (int) value; }
  long   longValue() { return (long) value; }
  short  shortValue() { return (short) value; }
}

class Short extends Number
{
  short value;

  public Short( short value )
  {
    this.value = value;
  }

  byte   byteValue() { return (byte) value; }
  double doubleValue() { return (double) value; }
  float  floatValue() { return (float) value; }
  int    intValue() { return (int) value; }
  long   longValue() { return (long) value; }
  short  shortValue() { return (short) value; }
}

class Boolean
{
  boolean value;

  public Boolean( boolean value )
  {
    this.value = value;
  }

  boolean booleanValue() { return value; }
}

class Character
{
  char value;

  public Character( char value )
  {
    this.value = value;
  }

  char charValue() { return value; }
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

