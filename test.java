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
    String st2 = "H-World";
    println( st2 );
    String st = "H";
    st += "-";
    st += "World";
    println( st == st2 );
    println( st.equals(st2) );
  }

  void holla$SEED() { println("woo"); }
}

class Bag<DataType>
{
  DataType n;

  public String toString() { return "Bag!"; }
}

