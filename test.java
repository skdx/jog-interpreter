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
		int[][] nums = {{1,2,3},{4,5,6}};
    for (int j=0; j<nums.length; ++j)
    {
      for (int i=0; i<nums[0].length; ++i)
      {
        if (i > 0) print(",");
        print( nums[j][i] );
      }
      println();
    }
  }

  int random100()
  {
    return (int)(Math.random()*100);
  }
}
