#include "LaplacianApp.hpp"

LaplacianApp::LaplacianApp(int argc, char* argv[]): cli_interface{"Allowed options"} {
  cli_interface.add_options()
    ("help,h", "This help message")
    ("directory,d", cli::value<fs::path>(),"Where this program should look for pictures to be analyzed")
    ("threads,t", cli::value<std::size_t>(), "How many worker threads to spawn")
    ("save-to,s", cli::value<fs::path>(), "Where csv file will be saved");
  cli::store(cli::parse_command_line(argc, argv,cli_interface), vm);
  cli::notify(vm);

  if(vm.count("help") || argc!=7){
    throw Error(std::move(cli_interface));
  }

  if(vm.count("directory")){
    directory_path = vm["directory"].as<fs::path>();
    std::cout << "Selected working directory is: " << directory_path << '\n';
  }

  if(vm.count("save-to")){
    csv_path = vm["save-to"].as<fs::path>();
    csv_path /= "laplacian.csv";
    std::cout << "CSV file with the analysis results will be saved to "
      << csv_path << '\n';
  }

  if(directory_path.empty() || csv_path.empty()){
    std::cout << "There are missing options. See help for more information.\n";
    throw Error(std::move(cli_interface));
  }

  auto max_thread_count = std::thread::hardware_concurrency();
  if(vm.count("threads") && vm["threads"].as<std::size_t>() < max_thread_count){
    thread_count = vm["threads"].as<std::size_t>();
  } else {
    thread_count = std::max(std::size_t(max_thread_count-2),std::size_t{2});
    std::cout << "Invalid option. Defaulting to " << thread_count << " threads.\n";
  }
}

// Program should only reach this point if the command line options are properly set.
int LaplacianApp::run(){
  asio::thread_pool pool{thread_count};

  for(const auto& entry : fs::directory_iterator(directory_path))
  {
    asio::dispatch(pool, [this, entry](){
        cv::Mat src, src_gray, laplacian, scaled, mean, stdDev, variance;
        src = cv::imread(entry.path());
        cv::cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
        cv::Laplacian(src_gray, laplacian, CV_64F);
        cv::convertScaleAbs(laplacian, scaled);
        cv::meanStdDev(scaled, mean, stdDev);
        cv::pow(stdDev,2, variance);
        auto meanVar = cv::mean(variance);

        std::lock_guard guard{variances_flatmap_mutex};
        std::cout << "==== Locking from thread " << std::this_thread::get_id() << '\n';
        thread_ids.insert(std::this_thread::get_id());
        variances_flatmap.emplace_back(entry.path().string(), meanVar[0]);
        });
  }

  pool.join();

  std::cout << thread_ids.size() << " threads were used to perform the jobs submitted to the pool.\n";

  std::sort(variances_flatmap.begin(), variances_flatmap.end(), compare_by_second<Pair>);
  std::cout << "== [Top 5 variances] ==\n";
  auto vfiter = variances_flatmap.crbegin();
  for(int i=0; i<5; i++, vfiter++){
    std::cout << "File: " <<vfiter->first << "\t Mean Variance found: " << 
      vfiter->second << '\n';
  }
  std::cout << "== [Lowest 5 variances] ==\n";
  auto vfiterb = variances_flatmap.cbegin();
  for(int i=0; i<5; i++, vfiterb++){
    std::cout << "File: " <<vfiterb->first << "\t Mean Variance found: " << 
      vfiterb->second << '\n';
  }

  std::ofstream csv;
  csv.open(csv_path.c_str(), std::ios::ate);
  csv << "FilePath,LaplacianVariance\n";
  for(const auto& pair : variances_flatmap){
    csv << '\"' << pair.first << "\"," << pair.second << '\n';
  }
  csv.close();

  return 0;
}


