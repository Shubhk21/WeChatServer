#ifdef __APPLE__
#include "config.h"
#include "unix_server.h"
#include "db_manager.h"

int US::server_socket;

std::map<std::string, int> US::usr_to_soc;

std::map<int, std::string> US::soc_to_usr;

void US::createSocket(){
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        std::cerr << "Error: Creating socket \n";
        exit(-1);
    }

    std::cout<<"Server Socket is created! \n";
}

void US::bindSocket(){
    sockaddr_in server_info;
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(CONFIG::SERVER_PORT_SOCKET);

    if(inet_pton(AF_INET, CONFIG::SERVER_IP.c_str(), &server_info.sin_addr.s_addr) != 1){   //Error case
        std::cerr << "Error: Server configuration \n";
        close(server_socket);
        exit(-1);
    }

    if(bind(server_socket, (struct sockaddr*)&server_info, sizeof(server_info)) != 0){
        std::cerr << "Binding failed \n";
        close(server_socket);
        exit(-1);
    }

    std::cout<<"Server configured! \n";
}

void US::listenClient(){
    if(listen(server_socket, SOMAXCONN) != 0){
        std::cerr << "Listening failed \n";
        close(server_socket);
        exit(-1);
    }
}

int US::acceptClient(){
    return accept(server_socket, nullptr, nullptr);
}

static void parseAndSend(int sender_fd, const std::string& json_data){
    try
    {
        nlohmann::json data_packet = nlohmann::json::parse(json_data);

        std::string sender = data_packet["sender"];
        std::string recipient = data_packet["receiver"];
        std::string message_content = data_packet["data"];

        if(message_content == "hand_shake" && recipient == "hand_shake"){
            auto it = US::soc_to_usr.find(sender_fd);
            it->second = sender;
            US::usr_to_soc.emplace(sender, sender_fd);
            std::cout<< "handshake triggered! \n";
            return;
        }

        auto it = US::usr_to_soc.find(recipient);
        if(it == US::usr_to_soc.end()){
            nlohmann::json error_response = {
                {"sender", "server"},
                {"reciever", sender},
                {"data", "Error: User '" + recipient + "' not found"}
            };
            std::string error_json = error_response.dump();
            send(sender_fd, error_json.c_str(), error_json.length(), 0);
        }else{
            int recipient_id = it->second;

            send(recipient_id, json_data.c_str(), json_data.length(), 0);
        }
    }
    catch(const nlohmann::json::exception& e)
    {
        return;
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        
        // Send error back to sender
        nlohmann::json error_response = {
            {"sender", "server"},
            {"reciever", US::soc_to_usr[sender_fd]},
            {"data", "Error: Invalid message format"}
        };

        std::string error_json = error_response.dump();
        
        send(sender_fd, error_json.c_str(), error_json.length(), 0);
    }
}

void US::handleSocket(){
    try
    {
        int kq = kqueue();
        if(kq == -1){
            std::cerr << "kqueue failed \n";
            close(server_socket);
            exit(1);
        }

        //adding server to kqueue to watch incoming events:
        struct kevent server_add_event;
        EV_SET(&server_add_event, server_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);

        if(kevent(kq, &server_add_event, 1, NULL, 0, NULL) == -1){
            std::cerr << "kqueue add server socket failed \n";
            close(server_socket);
            close(kq);
            exit(1);
        }

        //Array to receive events:
        const int MAX_EVENTS=10;
        struct kevent input_events[MAX_EVENTS];

        while(1) {
            int total_events = kevent(kq, NULL, 0, input_events, MAX_EVENTS, NULL);

            if(total_events == -1){
                std::cerr << "kevent wait failed \n";
                close(server_socket);
                close(kq);
                exit(1);
            }

            //processing each event:
            for(int i=0; i<total_events; ++i){
                if(input_events[i].ident == server_socket){
                    //new client connection handler:
                    int client_fd = accept(server_socket, nullptr, nullptr);

                    if(client_fd == -1){
                        std::cerr << "Accept block for a client \n";
                        continue;
                    }

                    //make the socket non-blocking!
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
                    
                    //creating a struct for the client:
                    struct kevent client_event;
                    EV_SET(&client_event, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

                    if(kevent(kq, &client_event, 1, NULL, 0, NULL) == -1){
                        std::cerr << "Client was not added to KQueue \n";
                        close(client_fd);
                        continue;
                    }

                    //creating an entry in the map:
                    soc_to_usr.emplace(client_fd, "unknown");

                    std::cout<< "Client added to kqueue! \n";
                }else{
                    //client message handler:
                    std::cout<< "Server is receiving your messages! \n"; 
                    int client_fd = input_events[i].ident;
                    
                    char buffer[1024];
                    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

                    if(bytes_read > 0) {
                        buffer[bytes_read] = '\0';
                        parseAndSend(client_fd, buffer);
                        std::cout << "Received message from user: " << soc_to_usr[client_fd] << std::endl;
                    }else if(bytes_read <= 0) {
                        std::cout << "Client disconnected" << std::endl;
                        LDB::is_active_mutex.lock();
                        LDB::currently_active.erase(soc_to_usr[client_fd]);
                        LDB::is_active_mutex.unlock();
                        auto it = soc_to_usr.find(client_fd);
                        usr_to_soc.erase(it->second);
                        soc_to_usr.erase(client_fd);
                        close(client_fd);
                    }
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return;
    }
    
}

#endif
