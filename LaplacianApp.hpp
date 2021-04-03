#include <iostream>
#include <thread>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/dispatch.hpp>
#include "opencv2/opencv.hpp"

namespace{
  namespace cli = boost::program_options;
  namespace asio = boost::asio;
  namespace fs = std::filesystem;
}

using Pair = std::pair<std::string, double>;
class LaplacianApp {
  private:
    cli::options_description cli_interface;
    cli::variables_map vm;

    std::mutex variances_flatmap_mutex;
    std::vector<Pair> variances_flatmap;
    fs::path directory_path, csv_path;
    std::set<std::thread::id> thread_ids;
    std::size_t thread_count;



  public:
    template <typename PairT>
      static bool compare_by_second(PairT p1, PairT p2){
        return (p1.second < p2.second);
      }

    class Error {
      private:
        cli::options_description desc;

      public:
        Error(cli::options_description&& error) : desc(error){};
        auto what(){
          return desc;
        }
    };

    int run();
    LaplacianApp(int argc, char* argv[]);
};
