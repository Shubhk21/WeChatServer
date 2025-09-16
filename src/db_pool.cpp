#include "db_pool.h"

DBPool::DBPool(const std::string &conninfo, int poolSize)
{

    for (int i = 0; i < poolSize; i++)
    {

        PGconn *conn = PQconnectdb(conninfo.c_str());
        if (PQstatus(conn) != CONNECTION_OK)
        {
            throw std::runtime_error("Connection failed: " + std::string(PQerrorMessage(conn)));
        }
        connections.push(conn);
    }
}

DBPool::~DBPool()
{
    while (!connections.empty())
    {
        PQfinish(connections.front());
        connections.pop();
    }
}

PGconn *DBPool::acquireConnection()
{
    std::unique_lock<std::mutex> lock(mtx);
    cond.wait(lock, [this]()
              { return !connections.empty(); });
    PGconn *conn = connections.front();
    connections.pop();
    return conn;
}

void DBPool::releaseConnection(PGconn * conn)
{
    std::unique_lock<std::mutex> lock(mtx);
    connections.push(conn);
    lock.unlock();
    cond.notify_one();
}