#include "LaplacianApp.hpp"

int main(int argc, char* argv[]){
  try{
    LaplacianApp myApp(argc, argv);
    return myApp.run();
  }
  catch(LaplacianApp::Error& e){
    std::cout << e.what();
  }
}
