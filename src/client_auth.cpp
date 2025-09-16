#include "client_auth.h"
#include "db_pool.h"

void handleClientAuth(){

    DBPool DBPool_obj("host=localhost port=5432 dbname=postgres user=postgres password=210820",5);

    httplib::Server auth_server;

    auth_server.Post("/login", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received GET /getdata from client" << std::endl;

        
        // db call




        res.set_content(R"({"status": "bool"})", "application/json");
    });

    auth_server.Post("/register", [&DBPool_obj](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received GET /getdata from client" << std::endl;

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
            "INSERT INTO users(username,password) VALUES($1,$2)",
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


    auth_server.listen(CONFIG::SERVER_IP,CONFIG::SERVER_PORT);

}
