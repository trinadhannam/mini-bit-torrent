#include<iostream> //basic inp outpuy
#include<stdio.h>
#include<string.h>
#include<sys/socket.h> //for socket creation
#include<arpa/inet.h> // in_addr structure
#include<sys/types.h> // defines various basic derived types
#include<errno.h>
#include<unistd.h> //read write
#include<cstring>
#include<string>
#include<pthread.h>
#include<fcntl.h>
#include<netdb.h>
#include<openssl/sha.h>
#include<bits/stdc++.h>
#include <netinet/in.h>
using namespace std;
typedef long long ll;

//to store all the information related the users groups and files

map<string, string> users;
map<string, string> user_ports;
map<string, bool> log_status;
map<string , vector<string>> group_members;
map<string , vector<string>> group_req;
map<string , vector<pair<string,string>>> group_files;
map<string , map<string, vector<string>>> file_info;
map<string , vector<pair<string,string>>> file_chunks;

vector<string> tokenize_string(string s,char c){
    vector<string> tokens;
    string t;
    stringstream ss(s);
    while(getline(ss,t,c)){  
        tokens.push_back(t);
    }
    // for(auto i : tokens){
    //     cout<<i<<' ';
    // }cout<<endl;
    return tokens;
}

string get_fname(string path){
    string filename ;
    size_t found ;
    //we need to find the last '/' in the path to get to the file name
    //caues from that '/' we have the file name

    string fname = "";
    int i = path.size() - 1;
    while(i>=0){
        if(path[i] == '/'){break;}
        fname += path[i];
        i--;
    }
    reverse(fname.begin(), fname.end());
    return fname;
    
    /*
    found = path.find_last_of('/');
    if(found != string::npos){
        //cout<<found<<endl;
        filename = path.substr(found+1);
        return filename;
    }
    else if(found == string::npos){
        filename = path;
    }
    return filename;
    */
}

void portinformation(vector<string> &track_info , string path){
    int fd = open(path.c_str(), O_RDONLY );
    char buffer[1024];
    
    off64_t fsize = lseek(fd, 0, SEEK_END);
    lseek64(fd, 0, SEEK_SET);
    
    read(fd , buffer , fsize);
    string s = buffer;
    string t;
    stringstream ss(s);
    while(getline(ss,t,' ')){  
        track_info.push_back(t);
    }    
}

int commandvalidation(string first , int n){
    if(first == "create_user" and n == 3 ){
        return 1;
    }
    else if(first == "login" and n==4){
        return 2;
    }
    else if(first == "create_group" and n==2){
        return 3;
    }
    else if(first == "join_group" and n==2){
        return 4;
    }
    else if(first == "leave_group" and n==2){
        return 5;
    }
    else if(first == "accept_request" and n==3){
        return 6;
    }
    else if(first == "list_requests" and n==2){
        return 7;
    }
    else if(first == "list_groups" and n==1){
        return 8;
    }
    else if(first == "list_files" and n==2 ){
        return 9;
    }
    else if(first == "upload_file" and n==4){
        return 10;
    }
    else if(first == "download_file" and n==4){
        return 11;
    }
    else if(first == "show_downloads" and n==1){
        return 12;
    }
    else if(first == "stop_sharing" and n==3){
        return 13;
    }
    else if(first == "logout" and n ==1){
        return 14;
    }
    else if(first == "list_members"  and n==2){
        return 15;
    }
    else if(first == "show_commands"){
        return 16;
    }
    else{
        return -1;
    }

}

void *tracker_connect(void *recv_client_socket){
    ll client_socket = *(ll*)recv_client_socket;

    int stdin = 0;
    char buffer[4096];
    string curr_user = "";
    string response;

    while(1){
        memset(buffer, 0, sizeof(buffer));
        ll bytes_received = recv(client_socket, buffer, sizeof(buffer), stdin);
        if (bytes_received <= 0) {
            perror("Connection closed or error");
            break;
        }
        //we need to get the parsed command using the tokenising the string
        vector<string> cmd = tokenize_string(buffer, ' ');
        int command = commandvalidation(cmd[0] , cmd.size());
        
        //CREATING A  USER
        if(cmd[0] == "quit"){
            response ="QUITING";

        }
        else if(command == -1){
            response = "ENTER VALID NUMBER OF ARGUMENTS";
        }
        else{

            if(command ==1){
                //create_user <user_id> <passwd>
                // if(cmd.size() != 3){
                //     response = "ENTER VALID NUMBER OF ARGUMENTS";
                // }
                
                if(users.find(cmd[1]) != users.end()){
                    response = "UESR ALREADY EXIST";
                }
                else{
                    //create user
                    users[cmd[1]] = cmd[2];
                    response = "USER CREATED";
                }
                
            }
            else if(command == 2){
                //login <user_id> <passwd> + <portno>
                // if(cmd.size() != 4){
                //     response = "ENTER VALID NUMBER OF ARGUMENTS";
                // }
                
                if(users.find(cmd[1]) == users.end()){
                    response = "USER DOESNT EXIST";
                }
                // else if(curr_user != "" or curr_user != cmd[1]){
                //     response = "THERE's AN USER ALREADY RUNNNING LOGOUT THE USER AND LOGIN AGAIN";
                // }
                else{
                    //login the user
                    //2 cases already if the uesr is alraedy logged in or not logged in
                    
                    if(log_status[cmd[1]] == true){
                        response = "USER ALREADY LOGGED IN ";
                    }
                    else{
                        if(users[cmd[1]] != cmd[2]){
                            response = "WRONG PASSWORD CANNOT LOGIN";
                        }
                        else{
                            log_status[cmd[1]] = true;
                            int n = cmd.size();
                            curr_user = cmd[1];
                            user_ports[cmd[1]] = cmd.back();
                            response = "USER LOGGED IN SUCCESFULLY ";
                            //cout<<cmd[1]<<' '<<' '<<user_ports[cmd[1]]<<endl;
                        }
                    }
                }
            
            }

            // else if(cmd[0] != "create_user" or cmd[0] != "login"){
            //     if(users.find(cmd[1]) == users.end() or log_status[cmd[1]] == false){
            //         response = "USER NOT LOGGED IN ";
            //     }
            // }

            else if(command == 3){
                //create_group <group_id>
                // if(cmd.size() != 2){
                //     response = "ENTER VALID NUMBER OF ARGUMENTS";
                // }
                // else{
                    if(group_members.find(cmd[1]) != group_members.end()){
                        response = "USER GROUP ALREADY CREATED";
                    }
                    else if(group_members.find(cmd[1]) == group_members.end()){
                        //the user who called the create_group command will become the owner of that group
                        group_members[cmd[1]].push_back(curr_user);
                        response = "USER GROUP CREATED";
                    }
                
            }

            else if(command == 4){
                // join_group <group_id>
                // if(cmd.size() != 2){
                //     response = "ENTER VALID NUMBER OF ARGUMENTS";
                // }
                // else{
                    if(group_members.find(cmd[1]) == group_members.end()){
                        response = "GROUP WITH THE GROUP ID DOESNT EXIST";
                    }
                    else{
                        //first we check whether the user which made the request is already presnet in the group or not
                        vector<string> allmems = group_members[cmd[1]];
                        bool present = false;
                        for(auto i : allmems){
                            if(i == curr_user){
                                present = true;
                            }
                        }
                        if(present){
                            response = "THE CURRENT USER IS ARLEADY IN THE GROUP";
                        }
                        else{
                            //we need to add the request to the group id;
                            group_req[cmd[1]].push_back(curr_user);
                            response = "CURRENT USER QUEUED UP FOR ACCEPTING THE REQUEST";
                        }
                    }
                
            }

            else if(command == 5){
                // leave_group <group_id>
                string group_id = cmd[1];
                //first we check if the group exists or not
                if(group_members.find(group_id) == group_members.end()){
                    response = " GROUP DOES NOT EXIST";
                }
                else{
                    //then we check if the curr_user is the part of the group or not
                    vector<string> allmems = group_req[cmd[1]];
                    bool present = false;
                    for(auto i : allmems){
                        if(i == cmd[2]){
                            present = true;
                        }
                    }
                    if(!present){
                        response = "USER IS NOT THE PART OF THE GROUP";
                    }
                    else{
                        //now we remove the user from the group
                        if(group_members.size()==1){
                            //only one person in this group soo remove everything related to this group
                            group_members.erase(group_id);
                            group_files.erase(group_id);
                            file_info.erase(group_id);
                            group_req.erase(group_id);
                            response = "GROUP REMOVED SUCCESSFULLY";
                        }
                        else{
                            vector<string> newmems;
                            for(auto i : allmems){
                                if(i == curr_user){
                                    continue;
                                }
                                else{
                                    newmems.push_back(i);
                                }
                            }
                            group_members[group_id] = newmems;
                            response = "USER LEFT THE GROUP SUCCESSFULLY";

                        }
                    }
                }
            }

            else if(command == 6 ){
                //accept_request <group_id> <user_id>
                // if(cmd.size() != 3){
                //     response = "ENTER VALID NUMBER OF ARGUMENTS";
                // }
                // else{
                if(group_req.find(cmd[1]) == group_req.end()){
                    response = "GROUP DOESNT EXITS";
                }
                else{
                    vector<string> allmems = group_req[cmd[1]];
                    bool present = false;
                    for(auto i : allmems){
                        if(i == cmd[2]){
                            present = true;
                        }
                    }
                    if(!present){
                        response = "USER DIDNOT MAKE A REQUEST TO THE GROUP";
                    }
                    else{
                        //the user who is requesting from other end is present in the request map
                        //now firstly wee need to check wheter this guy is the owner of the group or not
                        if(group_members[cmd[1]][0] != curr_user){
                            response = "YOU ARE NOT THE OWNER OF THIS GROUP SO YOU CANNOT ACCEPT THE REQUESTS";
                        }                      
                        else{
                            //now accept the request and push it into the group_memebrs
                            group_members[cmd[1]].push_back(cmd[2]);
                            allmems.erase(remove(allmems.begin(), allmems.end() , cmd[2]),allmems.end());
                            response = "REQUEST ACCEPTED BY GROUP";
                        }
                    }
                }
                
            }

            else if(command == 7){
                //list_requests <group_id>
                //firstly we find whether the group is present or not then we check for the pending requests in the group
                if(group_members.find(cmd[1]) == group_members.end()){
                    response = " GROUP DOES NOT EXIST";
                }
                else{
                    vector<string> allreq = group_req[cmd[1]];
                    if(group_members[cmd[1]][0] == curr_user){
                        response = "REQUESTS FOR THE GROUP ARE : \n";
                        for(auto i : allreq){
                            response += i+"\n";
                        }
                    }
                    else{
                        response ="YOU ARE NOT THE OWNER OF THIS GROUP";
                    }
                }
            }
            
            else if(command == 8){
                response = "All the groups are :  \n";
                for(auto i : group_members){
                    response += i.first + "  ";
                }
            }

            else if(command == 15){
                //list members <group_id>
                if(cmd.size() != 2){
                    response = "ENTER VALID NUMBER OF ARGUMENTS";
                }
                else{
                    vector<string> allmems = group_members[cmd[1]];
                    response = "All the members in the group"+cmd[1]+"are : \n";
                    for(auto i : allmems){
                        response += i + " ";
                    }   
                }
            }

            else if(command == 9){
                //list_files <group_id>

                if(group_members.find(cmd[1]) == group_members.end()){
                    response = "GROUP DOESNT EXIST";
                }
                else{
                    response = "All the files in the group "+cmd[1]+" are : \n";
                    vector<pair<string,string>> allfileinfo = group_files[cmd[1]];
                    for(auto i : allfileinfo){
                        response += i.first + " ";
                    }
                }
            }


            else if(command == 10){
                //upload_file <file_path> <group_id>
                //but when we receive the msg grom the client it will also contain the SHAfile string
                //upload_file <file_path> <group_id> <SHA_for_file> <chunknumber> 
                if(cmd.size() != 4){
                    response = "ENTER VALID NUMBER OF ARGUMENTS";
                }
                else{
                    string group_id = cmd[2];
                    if(group_members.find(group_id)==group_members.end()){
                        response = "GROUP DOESNT EXIST";
                    }
                    else{
                        vector<string> allmems = group_members[group_id];
                        bool present = false;
                        for(auto i : allmems){
                            if(i == curr_user){
                                present = true;
                            }
                        }
                        if(!present){
                            response = "USER IS NOT THE PART OF THE GROUP TO MAKE THE UPLOAD";
                        }
                        else{
                            string fname = get_fname(cmd[1]);
                            string fileSHA = cmd[3];
                            /*
                            map<string , vector<pair<string,string>>> group_files;
                            - This is used for searching the files in the particular group with the group id
                            and retrives the file name and its respective SHA code associated with the file
                            */
                            
                            group_files[group_id].push_back(make_pair(fname , fileSHA));

                            /*
                            map<string , map<string, vector<string>>> file_info;
                            - for every group we maintain a file information
                            - Then it takes the name of the file and the users having the file  
                            */
                        file_info[group_id][fname].push_back(curr_user);
                        response = "FILE UPLOADED SUCCESSFULLY";
                        }
                    }
                }
            }

        
            else if(command == 11){
                //download_file <group_id> <file_name> <destination_path>
                /*
                - find if the group_id exists or not
                - find if the curr_user is the part of that group to accesss the files or not
                - Then find if file wiht fname exists in that group or not
                - If file found then give the port of the user that have the file and send the response to the client
                */
                if(cmd.size() != 4){
                    response = "ENTER VALID NUMBER OF ARGUMENTS";
                }
                else{
                    string group_id = cmd[1];
                    string fname = cmd[2];
                    if(group_members.find(group_id) == group_members.end()){
                        response = "GROUP DOESNT EXIST FOR DOWNLOADING";
                    }
                    else{
                        vector<string> allmems = group_members[group_id];
                        bool present = false;
                        for(auto i : allmems){
                            if(i == curr_user){
                                present = true;
                            }
                        }
                        if(!present){
                            response = "USER IS NOT THE PART OF THE GROUP TO MAKE THE DOWNLOAD";
                        }
                        else{
                            bool filepresent = false;
                            for(auto i : group_files[group_id]){
                                if(i.first == fname){
                                    filepresent = true;
                                    break;
                                }
                            }
                            if(filepresent == false){
                                response = "FILE REQUESTED TO DOWNLOAD IS NOT PRESENT IN THIS GROUP";
                            }
                            else{
                                response = "";
                                vector<string> fileinfo = file_info[group_id][fname];
                                for(auto i : fileinfo){
                                    if(log_status[i] == true){
                                        response += (user_ports[i]+" ");
                                    }
                                }
                            }
                        }
                    }
                }
            }

            else if(command == 12){
                response = "SHOWING DOWNLOADS";
            }

            else if(command == 14){
                log_status[curr_user] = false;
                user_ports.erase(curr_user);
                curr_user = "";
                response = "USER LOGGED OUT SUCCESSFULLY";
            }

            else if(command == 16){
                cout<<"create_user <user_id> <passwd>"<<endl;
                cout<<"login <user_id> <passwd>"<<endl;
                cout<<"create_group <group_id>"<<endl;
                cout<<"join_group <group_id>"<<endl;
                cout<<"leave_group <group_id>"<<endl;
                cout<<"list_requests <group_id>"<<endl;
                cout<<"accept_request <group_id> <user_id>"<<endl;
                cout<<"list_groups"<<endl;
                cout<<"list_files <group_id>"<<endl;
                cout<<"upload_file <file_path> <group_id>"<<endl;
                cout<<"download_file <group_id> <file_name> <destination_path>"<<endl;
                cout<<"show_downloads"<<endl;
                cout<<"stop_share <group_id> <file_name>"<<endl;
                cout<<"logout"<<endl;
                cout<<endl;
            }
        }
        cout<<response<<endl<<endl;
        send(client_socket, response.c_str(), response.size(), 0);
    }
    close(client_socket);
}


int main(int argc, char **argv){
    ll server_socket, client_socket;
    sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    vector<string> information;
    string path = argv[1];
    portinformation(information , path);


    string tracker_ip_port = information[0];

    // Set up server address
    ll portno = stoi(information[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);  // Change to your desired port
    server_addr.sin_addr.s_addr = INADDR_ANY;

 
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listening failed");
        exit(1);
    }

    cout << "Server is listening on port..."<<portno<< endl;

    // Accept connections from clients
    while(1){
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("Acceptance failed");
            exit(1);
        }
        pthread_t newthread;
        pthread_create(&newthread, NULL , tracker_connect ,&client_socket);
    }
    

    // Close sockets
    //close(client_socket);
    close(server_socket);


}
bool check_command(string s){
    if (s == "download_file" || s == "login" || s == "upload_file" || s == "create_user" || s == "accept_request" || s == "stop_share" || s == "create_group" || s == "join_group" || s == "leave_group" || s == "list_requests" || s == "list_files" || s == "list_groups" || s == "logout" || s == "show_downloads")
        return true;
    else
        return false;
}

/*
ll server_socket, client_socket;
    sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set up server address
    ll portno = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);  // Change to your desired port
    server_addr.sin_addr.s_addr = INADDR_ANY;


    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listening failed");
        exit(1);
    }

    cout << "Server is listening on port..."<<portno<< endl;

    // Accept connections from clients
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_socket == -1) {
        perror("Acceptance failed");
        exit(1);
    }
    ll stdin = 0;
    while (true) {
        // Receive data from client
        memset(buffer, 0, sizeof(buffer));
        ll bytes_received = recv(client_socket, buffer, sizeof(buffer), stdin);
        if (bytes_received <= 0) {
            perror("Connection closed or error");
            break;
        }
        
        cout << "Client: " << buffer << endl;

        // Send a reply to client
        string reply;
        cout << "You: ";
        getline(cin, reply);
        send(client_socket, reply.c_str(), reply.size(), 0);
    }






    const char *filename = "received.txt";
    int writefile = open(filename, O_RDWR| O_CREAT |O_TRUNC|__O_LARGEFILE , 0600);

    off64_t filesize ;
    read(client_socket , &filesize , sizeof(off64_t));

    off64_t chunksize ;
    if(filesize >4096){
        chunksize = 4096;
    }
    else{
        chunksize = 1;
    }
    char *receivedbuffer = (char *) malloc(sizeof(char) * chunksize);

    while(1){
        
        read(client_socket , receivedbuffer, chunksize);
        write(writefile , receivedbuffer , chunksize);
    }
    cout<<"DATA RECEIVED AND SAVED"<<endl;

*/