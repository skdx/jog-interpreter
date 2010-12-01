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
//
//   TODO
//     - 'for-each'
//     - Multidimensional Arrays
//     - Respect private/protected access qualifiers
//     - Maybe inner classes, maybe exceptions

class Test
{
  Test()
  {
    println( 3 > 5 ? "totally true" : "falsivity" );
    // Ternary conditional is working, but needs a bit more work still to enable
    // autoboxing.
  }

  void juice( Fruit f ) { println( f.juice() ); }
}

interface Fruit
{
  public String juice();
}

class Apple implements Fruit
{
  public String juice()
  {
    return "apple juice";
  }
}

class Orange implements Fruit
{
  public String juice() { return "orange juice"; }
}

