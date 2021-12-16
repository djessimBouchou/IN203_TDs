std::chrono::time_point < std::chrono::system_clock > start, end;
start = std::chrono::system_clock::now();
...
end = std::chrono::system_clock::now();
std::chrono::duration < double >elapsed_seconds = end - start;