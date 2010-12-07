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
    ArrayList<Integer> nums = new ArrayList<Integer>();
    nums.add(12);
    nums.add(7);
    nums.add(20);
    nums.add(10);
    for (int n : nums) println(n);
  }
}

class Alpha
{
  void report() { println("Alpha"); }
}

class Beta extends Alpha
{
  void report() 
  { 
    super.report();
    println("Beta"); 
  }
}

