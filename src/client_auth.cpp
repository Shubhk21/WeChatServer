#include "client_auth.h"
#include "db_pool.h"

void handleClientAuth(){

    DBPool DBPool_obj("",5);

    httplib::Server auth_server;

    auth_server.Post("/login", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received GET /getdata from client" << std::endl;

        
        // db call


        res.set_content(R"({"status": "bool"})", "application/json");
    });

    auth_server.Post("/register", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received GET /getdata from client" << std::endl;


        

        res.set_content(R"({"status": "bool"})", "application/json");
    });


    auth_server.listen(CONFIG::SERVER_IP,CONFIG::SERVER_PORT);

}
