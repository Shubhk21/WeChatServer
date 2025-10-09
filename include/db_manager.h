#include<set>
#include<string>
#include<mutex>

namespace LDB {
    extern  std::set<std::string> currently_active;
    extern  std::mutex is_active_mutex;
}