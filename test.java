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
//     - Generics (actually templates are used, but it looks the same)
//     - Autoboxing
//     - Interfaces
//     - Inner classes (kind of - their names are still global)
//     - Multidimensional Arrays
//     - 'for-each'
//
//   TODO
//     - Respect private/protected access qualifiers
//     - Exceptions

class Test
{
	Test()
	{
    String st = " Happy Days ";
    st = st.trim();
    println( st );
    println( st.toLowerCase() );
    println( st.toUpperCase() );
    println( st.indexOf('D') );
    println( st.substring(st.indexOf('D')) );
    println( st.indexOf("Da") );
    println( st.substring(st.indexOf("Da")) );
    println( st.indexOf('p') );
    println( st.lastIndexOf('p') );
    println( st.startsWith("appy") );
    println( st.endsWith("Day") );
    println( st.replaceFirst("p","b") );
    println( "abc".compareTo("abc") );
    println( "abe".compareToIgnoreCase("ABE") );
    println( "xyz".charAt(1) );
  }

  native static public void printSound( float[] data );
  native static public void playSound( float[] data );
  native static public void printImage( byte[] data );

  static public void printImage( byte[][][] data )
  {
    int w = data.length;
    int h = data[0].length;
    byte[] packed = new byte[w*h*3];

    int pos = 0;
    for (int j=0; j<h; ++j)
    {
      for (int i=0; i<w; ++i)
      {
        packed[pos++] = data[i][j][0];
        packed[pos++] = data[i][j][1];
        packed[pos++] = data[i][j][2];
      }
    }

    printImage( packed );
  }
}

public class Animal
{
  protected String name;

  public Animal( String name ) { this.name = name; }
  public String toString() { return name; }
  public void speak() { println("Neigh!"); }
}

public interface Transportation
{
  public double speed();
}

public class Horse extends Animal implements Transportation
{
  public Horse( String name )
  {
    super(name);
  }

  public double speed() { return 30.0; }
}

