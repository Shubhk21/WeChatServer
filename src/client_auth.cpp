#include "client_auth.h"
#include "db_pool.h"


void handleClientAuth(){

    DBPool DBPool_obj(CONFIG::DB_URL,5);

    httplib::Server auth_server;

    auth_server.Post("/login", [&DBPool_obj](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received POST /login from CLIENT" << std::endl;

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

        const char* query = "SELECT id FROM users WHERE username = $1 AND password_hash = $2";
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

    auth_server.Post("/register", [&DBPool_obj](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received POST /register from client" << std::endl;

        auto j = nlohmann::json::parse(req.body);

        std::string username = j.at("username").get<std::string>();
        std::string password = j.at("password").get<std::string>();
        std::string secret_key = j.at("secretKey").get<std::string>();

        if(secret_key!=CONFIG::secret_key){
            res.status = 403;
            return;
        }

        PGconn * conn = DBPool_obj.acquireConnection();

        const char* paramValues[2];

        paramValues[0] = username.c_str();
        paramValues[1] = password.c_str();

        PGresult* check_result = PQexecParams(
            conn,
            "SELECT id FROM users WHERE username = $1",
            1,        // number of parameters
            NULL,     // let PostgreSQL infer types
            paramValues,
            NULL,     // lengths (not needed for text)
            NULL,     // formats (NULL = all text)
            0         // text results
        );
        
        int rows = PQntuples(check_result);
        PQclear(check_result);
        if(rows>0){
            res.status = 409;
        }
        else{
            PGresult* insert_result = PQexecParams(
            conn,
            "INSERT INTO users(username,password_hash) VALUES($1,$2)",
            2,        // number of parameters
            NULL,     // let PostgreSQL infer types
            paramValues,
            NULL,     // lengths (not needed for text)
            NULL,     // formats (NULL = all text)
            0         // text results
            );

            if(PQresultStatus(insert_result) == PGRES_COMMAND_OK){
                res.status = 200;
            }
            else{
                res.status = 500;
            }
            PQclear(insert_result);
        }
        DBPool_obj.releaseConnection(conn);
    });


    auth_server.listen(CONFIG::SERVER_IP,CONFIG::SERVER_PORT_API);

}
