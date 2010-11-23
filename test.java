// Project state:
//   WORKING
//     - Classes, extended classes, polymorphism
//     - this() and super() constructor calls, super.method() calls
//     - Class methods, object methods, overloads, overrides
//     - Class and object property initial value assignments, static initialization blocks
//     - Primitive types and operations
//     - Object types
//     - System.out.print/println for primitive types
//     - Arrays (1D)
//     - Strings
//     - Generics (actually templates are used)
//
//   TODO
//     - 'for-each'
//     - Interfaces
//     - Multidimensional Arrays
//     - Autoboxing
//     - Respect private/protected access qualifiers
//     - Maybe inner classes, maybe exceptions

class Test
{

  Test()
  {
    println( "Hello World! " + 123 );
    ArrayList<String> names = new ArrayList<String>(1);
    names.add("Abe");
    names.add("James");
    names.add("Ty");
    names.add("Murphy");
    for (int i=0; i<names.size(); ++i)
    {
      println( names.get(i) );
    }
  }
}

class Bag<DataType>
{
  DataType n;

  public String toString() { return "Bag!"; }
}

//=============================================================================
// STANDARD LIBRARY - DON'T DELETE
//=============================================================================

//=============================================================================
//  ArrayList
//=============================================================================
class ArrayList<DataType>
{
  DataType[] data;
  int size;

  ArrayList()
  {
    this(10);
  }

  ArrayList( int capacity )
  {
    data = new DataType[capacity];
  }

  int size() { return size; }

  void ensureCapacity( int min_capacity )
  {
    if (data.length >= min_capacity) return;

    int new_cap = data.length * 2;
    if (new_cap < min_capacity) new_cap = min_capacity;
    DataType[] new_data = new DataType[new_cap];
    for (int i=0; i<size; ++i) { new_data[i] = data[i]; }

    data = new_data;
  }

  boolean add( DataType value )
  {
    ensureCapacity(size+1);
    data[size++] = value;
    return true;
  }

  DataType get( int index )
  {
    return data[index];
  }
}

//=============================================================================
//  Math
//=============================================================================
class Math
{
  native static double floor( double n );
}

//=============================================================================
//  Number + associated
//  - Byte, Double, Float, Integer, Long, Short, Boolean, Character
//=============================================================================

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
  static String toString( byte b )
  {
    return Integer.toString(b);
  }

  // PROPERTIES
  byte value;

  // METHODS
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

  String toString() { return toString(value); }
}

class Double extends Number
{
  // CLASS METHODS
  static String toString( double n )
  {
    // This simple implementation prints out the full number with
    // exactly 4 digits after the decimal point.  It uses
    // a 'long' to store the whole and fractional parts of the
    // number, so numbers larger than +/- 2E63 will not convert
    // correctly.
    boolean is_negative = false;
    if (n < 0)
    {
      is_negative = true;
      n = -n;
    }

    long whole = (long) Math.floor(n);

    n = n - Math.floor(n);
    n *= 10000.0;
    if (n - Math.floor(n) >= 0.5) n += 1.0;  // round off
    long decimal = (long) Math.floor(n);

    String whole_st = Long.toString(whole);
    if (is_negative) whole_st = "-" + whole_st;

    String decimal_st = Long.toString(decimal);
    while (decimal_st.length() < 4) decimal_st += "0";

    return whole_st + "." + decimal_st;
  }

  // PROPERTIES
  double value;

  // METHODS
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

  String toString() { return toString(value); }
}

class Float extends Number
{
  // CLASS METHODS
  static String toString( float n ) { return Double.toString(n); }

  // PROPERTIES
  float value;

  // METHODS
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

  String toString() { return toString(value); }
}

class Integer extends Number
{
  // CLASS METHODS
  static String toString( int n )
  {
    if (n == 0) return "0";
    if (n == 0x80000000) return "-2147483648";

    StringBuilder buffer = new StringBuilder();
    boolean is_negative = false;
    if (n < 0)
    {
      is_negative = true;
      n = -n;
    }

    while (n > 0)
    {
      buffer.append( (char)((n%10)+'0') );
      n /= 10;
    }
    if (is_negative) buffer.append('-');
    
    return buffer.reverse().toString();
  }

  // PROPERTIES
  int value;

  // METHODS
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

  public String toString()
  {
    return toString(value);
  }
}

class Long extends Number
{
  // CLASS METHODS
  static String toString( long n )
  {
    if (n == 0) return "0";
    if (n == 0x8000000000000000L) return "-9223372036854775808";

    StringBuilder buffer = new StringBuilder();
    boolean is_negative = false;
    if (n < 0)
    {
      is_negative = true;
      n = -n;
    }

    while (n > 0)
    {
      buffer.append( (char)((n%10)+'0') );
      n /= 10;
    }
    if (is_negative) buffer.append('-');
    
    return buffer.reverse().toString();
  }

  // PROPERTIES
  long value;

  // METHODS
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

  public String toString()
  {
    return toString(value);
  }
}

class Short extends Number
{
  // CLASS METHODS
  static String toString( short n ) { return Integer.toString(n); }

  // PROPERTIES
  short value;

  // METHODS
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

  String toString() { return toString(value); }
}

class Boolean
{
  // CLASS METHODS
  static String toString( boolean b )
  {
    if (b) return "true";
    return "false";
  }

  // PROPERTIES
  boolean value;

  // METHODS
  public Boolean( boolean value )
  {
    this.value = value;
  }

  boolean booleanValue() { return value; }

  String toString() { return toString(value); }
}

class Character
{
  static String[] ascii_strings =
  {
    "", "", "", "", "", "", "", "",
    "\t", "\n", "", "", "\r", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    " ", "!", "\"", "#", "$", "%", "&", "'",
    "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "{", "|", "}", "~", "" 
  };

  // CLASS METHODS
  static String toString( char ch )
  {
    if (ch < 128)
    {
      String result = ascii_strings[ch];
      if (result == null)
      {
        result = new String( new char[]{ch} );
        ascii_strings[ch] = result;
      }
      return result;
    }
    else
    {
      return new String( new char[]{ch} );
    }
  }

  // PROPERTIES
  char value;

  // METHODS
  public Character( char value )
  {
    this.value = value;
  }

  char charValue() { return value; }

  String toString() { return toString(value); }
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
    if (other == null) return concat("null");

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

  public StringBuilder reverse()
  {
    int i1=0, i2=size-1;
    while (i1 < i2)
    {
      char temp = data[i1];
      data[i1] = data[i2];
      data[i2] = temp;
      ++i1;
      --i2;
    }
    return this;
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

