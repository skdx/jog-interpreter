#include "jog.h"
int num_objects = 0;

int main()
{
  Ref<JogVM> vm = new JogVM();
  //Ref<JogScanner> scanner = new JogScanner(new JogReader("test.java"));

  try
  {
    vm->parse("libraries/jog/jog_stdlib.java");

    vm->parse( "test.java" );
    /*
    vm->parse( "hello.java",
        "public class Hello { Hello(){ println(\"Hello World!\\n\"); } }"
        );
    */

    vm->compile();
    vm->run("Test");
    //while (scanner->peek()->type != TOKEN_EOF)
    //{
      //scanner->read()->print();
    //}
  }
  catch (Ref<JogError> error)
  {
    error->print();
  }
  catch (...)
  {
    fprintf( stderr, "[Internal compiler error]\n" );
  }

  return 0;
}

