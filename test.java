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
//     - Nested classes (kind of - their names are still global)
//     - Multidimensional Arrays
//
//   TODO
//     - 'for-each'
//     - Respect private/protected access qualifiers
//     - Maybe inner classes, maybe exceptions

class Test
{
  Test()
  {
    println( 3 > 5 ? "totally true" : "falsivity" );
    // Ternary conditional is working, but needs a bit more work still to enable
    // autoboxing.

    overloadTest();

    new Whatever();

    int[][] nums = new int[3][2];
    nums[0][0] = 10;
    nums[0][1] = 11;
    nums[1][0] = 12;
    nums[1][1] = 13;
    nums[2][0] = 14;
    nums[2][1] = 15;
    println( nums[0][0] );
    println( nums[0][1] );
    println( nums[1][0] );
    println( nums[1][1] );
    println( nums[2][0] );
    println( nums[2][1] );
    int[] subset = nums[1];
    println( subset[0] );
    println( subset[1] );
  }

  class Whatever
  {
    Whatever() { println("Boo!"); }
  }

	void overloadTest()
	{
	  test( 5, 6 );  // prints: Two ints!
	  test( 5.0, 6.0 );  // prints: Two doubles!
	  test( 5, 6.0 );  // prints: Two doubles!
	}

	void test( int a, int b ) { println( "Two ints!" ); }
	void test( double a, double b ) { println( "Two doubles!" ); }
}
