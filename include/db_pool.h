#include <queue>
#include <mutex>
#include <condition_variable>
#include <libpq-fe.h>


class DBPool{

    private:
    std::queue<PGconn*> connections;
    std::mutex mtx;
    std::condition_variable cond;

    public:

    DBPool(const std::string& conninfo, int poolSize);

    ~DBPool();

    PGconn * acquireConnection();

    void releaseConnection(PGconn * conn);

};