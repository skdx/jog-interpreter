Jog is a lightweight but reasonably full-featured and fast Java compiler and interpreter.  Conceived for use in an educational Computer Science iPad app at Northern Arizona University, Jog may be useful for other projects as well.

Give Jog a try by cloning the Mercurial repository, editing "test.java", and running "make".

Jog has the following features:
  * Classes, extended classes, polymorphism
  * this() and super() constructor calls, super.method() calls
  * Class methods, object methods, overloads, overrides
  * Class and object property initial value assignments, static initialization blocks
  * Arrays (single and multidimensional)
  * Strings and common String methods
  * Generics (actually templates are used, but it looks the same for most code)
  * Autoboxing
  * Interfaces
  * Inner classes (kind of - their names are still global)
  * 'if', 'while', 'for', and 'for-each'
  * All operators
  * Java 6-style arraylists
  * A sampling of common API functionality (System.currentTimeMillis(), Math.abs(), etc.)
  * Global print() and println() support
  * Any number of classes per file may be defined
  * Does not require any external libraries

As well as the following current limitations:
  * 'switch' has not been implemented
  * Does not respect private/protected access qualifiers
  * Exceptions are not supported (you can use "assert(boolean,failMesg);")
  * The standard library is quite small
  * Code is compiled and executed from source with each run - there is no .class file support or other compiled code output