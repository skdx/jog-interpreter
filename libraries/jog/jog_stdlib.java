//=============================================================================
// Jog Standard Library
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
    assert( capacity>=1, "Initial ArrayList capacity must be at least 1." );
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
    assert( index>=0 && index<size, "ArrayList get() index out of bounds." );
    return data[index];
  }
}

//=============================================================================
//  Math
//=============================================================================
class Math
{
  static Random random_gen = new Random();
  static double PI = acos(-1.0);

  native static double cos( double radians );
  native static double sin( double radians );
  native static double tan( double radians );
  native static double acos( double n );
  native static double asin( double n );
  native static double atan( double slope );
  native static double atan2( double y, double x );

  native static double floor( double n );
  static double random() { return random_gen.nextDouble(); }
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

  static public void print( Object obj )
  {
    if (obj == null) System.out.print("null");
    else System.out.print(obj.toString());
  }

  static public void print( String st ) { System.out.print(st); }

  static public void println() { System.out.println(); }
  static public void println( boolean n ) { System.out.println(n); }
  static public void println( char ch ) { System.out.println(ch); }
  static public void println( double n ) { System.out.println(n); }
  static public void println( int n ) { System.out.println(n); }
  static public void println( long n ) { System.out.println(n); }

  static public void println( Object obj ) 
  { 
    if (obj == null) System.out.println("null");
    else System.out.println(obj.toString()); 
  }

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

  public boolean equals( String other )
  {
    if (this == other) return true;
    if (other == null || this.data.length != other.data.length) return false;

    int count = data.length;
    for (int i=0; i<count; ++i)
    {
      if (data[i] != other.data[i]) return false;
    }

    return true;
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

class PrintWriter
{
  void print( boolean n ) { print( Boolean.toString(n) ); }
  native void print( char ch );
  void print( double n ) { print( Double.toString(n) ); }
  void print( int n ) { print( Integer.toString(n) ); }
  void print( long n ) { print( Long.toString(n) ); }
  native void print( String st );

  void println() { print('\n'); }
  void println( boolean n ) { print(n); print('\n'); }
  void println( char ch ) { print(ch); print('\n'); }
  void println( double n ) { print(n); print('\n'); }
  void println( int n ) { print(n); print('\n'); }
  void println( long n ) { print(n); print('\n'); }
  void println( String st ) { print(st); print('\n'); }
}

class Random
{
  long seed;

  Random()
  {
    this( System.currentTimeMillis() );
  }

  Random( long seed )
  {
    this.seed = seed;
  }

  int next( int bits )
  {
    seed = (seed * 0x5DEECE66DL + 11L) & ((1L << 48) - 1L);
    return (int)(seed >>> (48 - bits));
  }

  boolean nextBoolean() { return next(1) == 1; }

  void nextBytes( byte[] bytes )
  {
    for (int i=0; i<bytes.length; ++i) bytes[i] = (byte) next(8);
  }

  double nextDouble()
  {
    return (((long)next(26) << 27) + next(27)) / (double)(1L << 53);
  }

  float nextFloat()
  { 
    return next(24) / ((float)(1 << 24));
  }

  /*
  double  next_next_gaussian;
  boolean have_next_next_guassian;
  public double nextGaussian() 
  {
    clear the have next flag in setSeed()
    if (have_next_next_guassian) 
    {
      have_next_next_guassian = false;
      return next_next_gaussian;
    } 
    else 
    {
      double v1, v2, s;
      do {
        v1 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
        v2 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
        s = v1 * v1 + v2 * v2;
      } while (s >= 1 || s == 0);
      double multiplier = StrictMath.sqrt(-2 * StrictMath.log(s)/s);
      next_next_gaussian = v2 * multiplier;
      have_next_next_guassian = true;
      return v1 * multiplier;
    }
  }
  */

  int nextInt() { return next(32); }

  public int nextInt( int limit ) 
  {
    assert( limit > 0, "nextInt() parameter is non-positive." );

    return (int)(nextDouble() * limit);
  }

  public long nextLong() { return ((long)next(32) << 32) + next(32); }

  public void setSeed( long seed )
  {
    this.seed = seed;
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

