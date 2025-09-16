#include "client_auth.h"
#include "db_pool.h"

void handleClientAuth(){

    DBPool DBPool_obj("postgresql://sani@localhost/testdb",5);

    httplib::Server auth_server;

    auth_server.Post("/login", [&DBPool_obj](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received POST request from CLIENT" << std::endl;

        if(req.body.empty()){
            nlohmann::json error_response = {
                {"message", "Request body cannot be empty"},
                {"status", "error"}
            };
            res.status=400;
            res.set_content(error_response.dump(), "application/json");
            return;
        }

        nlohmann::json request_data = nlohmann::json::parse(req.body);
        std::string username = request_data["username"];
        std::string password = request_data["password"];

        PGconn *conn = DBPool_obj.acquireConnection();

        const char* query = "SELECT id FROM users WHERE username = $1 AND password = $2";
        const char* paramValues[] = { 
            username.c_str(),
            password.c_str()
        };

        PGresult *db_response = PQexecParams(
            conn,
            query,
            2,
            nullptr,
            paramValues,
            nullptr,
            nullptr,
            0
        );

        if (PQresultStatus(db_response) != PGRES_TUPLES_OK) {
            std::cerr << "Query failed: " << PQerrorMessage(conn) << std::endl;
            nlohmann::json error_response = {
                {"message", "Internal server error occurred"},
                {"status", "error"}
            };
            res.status=500;
            res.set_content(error_response.dump(), "application/json");
            PQclear(db_response);
            DBPool_obj.releaseConnection(conn);
            return;
        }

        int numRows = PQntuples(db_response);

        if(numRows == 0){
            res.status = 401;
            nlohmann::json error_response = {
                {"message", "Invalid username or password"},
                {"status", "error"}
            };
            res.set_content(error_response.dump(), "application/json");
            PQclear(db_response);
            DBPool_obj.releaseConnection(conn);
            return;
        }

        nlohmann::json user_response = {
            {"message", "Login successful"},
            {"status", "ok"}
        };
        res.status=200;
        res.set_content(user_response.dump(), "application/json");
        PQclear(db_response);
        DBPool_obj.releaseConnection(conn);
    });

    auth_server.Post("/register", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received GET /getdata from client" << std::endl;


        

        res.set_content(R"({"status": "bool"})", "application/json");
    });


    auth_server.listen(CONFIG::SERVER_IP,CONFIG::SERVER_PORT);

}
