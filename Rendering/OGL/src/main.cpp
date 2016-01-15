#include "dimension.h"
#include <exception>
#include <iostream>
#include "renderer.h"

int main(int argc, char *argv[]) {
  try {
    Renderer<three_dimensional> renderer;

    renderer.begin();

  } catch(std::exception const& exception) {
      std::cout << "Aborting: " << exception.what() << std::endl;
      return 1;
  } catch(...) {
    std::cout << "Aborting: unknown exception" <<std::endl;
    return 1;
  }

  return 0;
}
