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
    /*
    ArrayList<Integer> nums = new ArrayList<Integer>();
    nums.add(12);
    nums.add(7);
    nums.add(20);
    nums.add(10);
    for (int n : nums) println(n);
    */

    Horse horse = new Horse("Zippy Chippy");

    // Incorrect result - will fix shortly.
    println(horse.toString());
  }
}

public class Animal
{
  protected String name;

  public Animal( String name ) { this.name = name; }
  public String toString() { return name; }
}

public interface Transportation
{
}

public class Horse extends Animal implements Transportation
{
  public Horse( String name )
  {
    super(name);
  }
}

