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

map<string,string> filename_and_path;
map<string , string> file_and_sha;
map<string, vector<string>> downloaded_files;

vector<string> tokenize_string(string s){
    vector<string> tokens;
    string t;
    stringstream ss(s);
    while(getline(ss,t,' ')){  
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

string calcSHA(string path){
    int file =open(path.c_str() , O_RDONLY);
    if(file<0){
        cout<<"UNABLE TO OPEN THE FILE FOR THE SHA256"<<endl;
        return "";
    }
    SHA256_CTX shafile;
    SHA256_Init(&shafile);
    
    char buffer[4096];
    size_t readb;
    
    while((readb = read(file , buffer , sizeof(buffer))) > 0){
        SHA256_Update(&shafile,buffer,readb);
    }

    close(file);
    
    unsigned char hashedfile[SHA256_DIGEST_LENGTH];
    SHA256_Final(hashedfile, &shafile);

    string sha256_file ;
    for(int i = 0; i<SHA_DIGEST_LENGTH;i++){
        char hex[3];
        sprintf(hex,"%02x",hashedfile[i]);
        sha256_file+= hex;
    }

    return sha256_file;
}

bool recv_file(string filename1 , string filepath , int socket_connection){
    string filename = filepath+'/'+filename1;
    int writefile = open(filename.c_str(), O_RDWR| O_CREAT |O_TRUNC|__O_LARGEFILE , 0777);
    if(writefile == -1){return false;}
    off64_t filesize ;
    read(socket_connection , &filesize , sizeof(off64_t));

    off64_t chunksize ;
    // if(filesize >4096){
    //     chunksize = 4096;
    // }
    // else{
    //     chunksize = 1;
    // }
    chunksize = 1;
    char *receivedbuffer = (char *) malloc(sizeof(char) * chunksize);

    while(1){
        //if(lseek64(readfile, 0 , SEEK_CUR)>filesize) break;
        int n ;
        if((n =read(socket_connection , receivedbuffer, chunksize) ) <= 0) break;
        
        write(writefile , receivedbuffer , chunksize);
    }
    cout<<"DATA RECEIVED AND SAVED"<<endl<<endl;
    return true;
}


void send_file(string filepath ,string fname , int socket_connection){
    int readfile ;
    const char *filename = filepath.c_str();
    readfile = open(filename , O_RDONLY | __O_LARGEFILE);

    off64_t filesize = lseek64(readfile, 0 , SEEK_END);
    write(socket_connection , &filesize , sizeof(off64_t));

    lseek64(readfile, 0 , SEEK_SET);

    off64_t chunksize ;
    // if(filesize >4096){
    //     chunksize = 4096;
    // }
    // else{
    //     chunksize = 1;
    // }
    chunksize =1;
    char *datafromfile = (char*)malloc(sizeof(char)*chunksize);

    while(1){
        int n ;
        if((n = read(readfile, datafromfile, chunksize))<=0) break;

        write(socket_connection, datafromfile,chunksize);
    }

    cout<<"DATA HAS BEEN TRANSFERRED SUCCESSFULLY"<<endl<<endl;

}

void* handle_cc(void* recv_client_socket){
    int client_socket = *(int*)recv_client_socket;

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, 2000,0);
    send_file(filename_and_path[buffer] ,buffer, client_socket);
    
    close(client_socket);
}


void* client_connections(void* recv_client_port){
    int client_port = *(int*)recv_client_port;

    int server_socket, client_socket;
    sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("CLIENT TO CLIENT Socket creation failed");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client_port);  // Change to your desired port
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("CLIENT TO CLIENT Binding failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("CLIENT TO CLIENT Listening failed");
        exit(1);
    }
    cout<<"client listening" <<endl;
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("Acceptance failed");
        exit(1);
    }
    // cout<<"CLIENT TO CLIENT CONNECTION IS ESTABLISHED"<<endl;
    // cout<<"NOW WE NEED TO SEND THE FILE THATS IT"<<endl;
    
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, 2000,0);
    send_file(filename_and_path[buffer] ,buffer, client_socket);
    
    cout<<file_and_sha[buffer]<<endl<<endl;
    cout<<"CLIENT"<<endl;
    close(client_socket);

    
    // pthread_t avail_threads[25];
    // int i = 0;
    // while(1){
    //     client_socket = accept(server_socket, NULL, NULL);
    //     if (client_socket == -1) {
    //         perror("Acceptance failed");
    //         exit(1);
    //     }
    //     cout<<"CLIENT TO CLIENT CONNECTION IS ESTABLISHED"<<endl<<endl;
    //     //pthread_create(&avail_threads[i++], NULL , handle_cc , &client_socket);
    //     char buffer[4096];
    //     memset(buffer, 0, sizeof(buffer));
    //     recv(client_socket, buffer, 2000,0);
    //     send_file(filename_and_path[buffer] , client_socket);
        
    //     close(client_socket);
    // }
    // for(int j = 0 ; j< i ; j++) {
    //     pthread_join(avail_threads[j], NULL);
    // }

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

int main(int argc, char **argv){
    ll client_socket;
    sockaddr_in server_addr;
    
    string client_ip_port = argv[1];
    int position = client_ip_port.find(':');
    string clientport = client_ip_port.substr(position+1);
    int finalclientport = stoi(clientport);
    cout<<"CLIENT WILL LISTEN AND ACCEPT FROM THE PORT:"<<clientport<<endl;

    pthread_t clientthread;
    pthread_create(&clientthread, NULL , client_connections, &finalclientport);


    vector<string> information;
    string path = argv[2];
    
    //get_info_from_port(information , path);
    portinformation(information ,path);

    string tracker_ip_port = information[0];
    int portno = stoi(information[1]);
    

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);  // Change to the port your server is listening on
    server_addr.sin_addr.s_addr = INADDR_ANY; // Change to the server's IP address
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    string message ;
    while(1){
        cout<<"CLIENT"<<endl;
        getline(cin,message);
        vector<string> tokens = tokenize_string(message);
        //string temp = message;
        if(tokens[0] == "login"){
            message += " "+clientport;
        }

        else if(tokens[0] == "upload_file"){
            if(argc != 3){
                cout<<"ERROR : Invalid arguments"<<endl;
                cout<<"upload_file <file_path> <group_id>"<<endl;
                continue;
            }
            else{
                string pathoffile = tokens[1];
                string filename = get_fname(pathoffile);
                cout<<"file obtained from the path given"<<endl;
                //now we calc the SHA of the file to be uploaded and append it to the sending string
                string sha256_for_file = calcSHA(tokens[1]);
                cout<<"SHA256 of file "<<' '<<sha256_for_file<<endl;
                message = tokens[0]+" "+filename+" "+tokens[2]+" "+sha256_for_file;
                filename_and_path[filename] = tokens[1];
                file_and_sha[filename] = sha256_for_file;
            }
        }


        send(client_socket,message.c_str(), message.size(), 0);
        memset(buffer, 0, sizeof(buffer));
        ll bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            perror("Connection closed");
            exit(1);
        }
        string response(buffer,bytes_received);
        cout<<response<<endl;
        cout<<endl;
        if(tokens[0] == "download_file" and response != "ENTER VALID NUMBER OF ARGUMENTS"){
            if(response.size() <= 0){
                cout<<"There are no clients to share this File"<<endl;
            }
            else{
                char obtained_sha[1024];
                vector<string> clientports = tokenize_string(response);
                if(clientports[0] !="GROUP" and clientports[0] !="USER" and clientports[0] !="FILE"){
                    for(auto i : clientports){
                        int client_server = socket(AF_INET, SOCK_STREAM, 0);
                    
                        int csportno = stoi(i);
                        struct sockaddr_in csaddr;
                        csaddr.sin_family = AF_INET;
                        csaddr.sin_port = htons(csportno);
                        csaddr.sin_addr.s_addr = INADDR_ANY;

                        int cs_connection = connect(client_server,(struct sockaddr *)&csaddr , sizeof(csaddr));
                        
                        if(cs_connection != -1){
                            cout<<"connected"<<' '<<cs_connection<<endl;
                            send(client_server , tokens[2].c_str() , 2000 , 0); 
                            //recv_file will take filename, destination path to store the file and , socket of the client
                        
                            if(recv_file(tokens[2] , tokens[3] , client_server)){
                                //we need to store the downloaded information of the file also
                                downloaded_files[tokens[1]].push_back(tokens[2]);
                                string csresponse = "FILE DOWNLOADED SUCCESSFULLY";
                                send(client_server , csresponse.c_str() , 2000 , 0);
                                memset(buffer , 0, sizeof(buffer));
                                recv(client_server , buffer , 2000 , 0);
                            }

                            close(client_server);
                        }
                        
                        break;
                    }
                    string resultfileobtained = tokens[3]+'/'+tokens[2];
                    string sha_for_result = calcSHA(resultfileobtained);
                    cout<<sha_for_result<<endl;
                }
            }
        }

        if(tokens[0] == "show_downloads" and response == "SHOWING DOWNLOADS"){
            if(downloaded_files.size() == 0){
                cout<<"NO CURRENT DOWNLOADS YET"<<endl;
            }
            else{
                for(auto i : downloaded_files){
                    cout<<"[D]"<<' '<<i.first<<' '<<"[C]"<<' '<<i.first<<endl;
                    for(auto j : i.second){
                        cout<<j<<' ';
                    }
                    cout<<endl;
                }
            }
        }
        if(response == "QUITING"){
            break;
        }
    }

    // Close the socket
    close(client_socket);

}




/*
    int fd = open(path.c_str(), O_RDONLY );
    char buffer[1024];
    off64_t fsize = lseek(fd, 0, SEEK_END);
    lseek64(fd, 0, SEEK_SET);

    while(1){
        read(fd , buffer , fsize);
        write(fd , buffer , fsize);
    }
    cout<<buffer<<endl;

    int readfile ;
    const char *filename = "input1.txt";
    readfile = open(filename , O_RDONLY | __O_LARGEFILE);

    off64_t filesize = lseek64(readfile, 0 , SEEK_END);
    write(client_socket , &filesize , sizeof(off64_t));

    lseek64(readfile, 0 , SEEK_SET);

    off64_t chunksize ;
    if(filesize >4096){
        chunksize = 4096;
    }
    else{
        chunksize = 1;
    }
    char *datafromfile = (char*)malloc(sizeof(char)*chunksize);

    while(1){
        read(readfile, datafromfile, chunksize);

        write(client_socket, datafromfile,chunksize);
    }

    cout<<"DATA HAS BEEN TRANSFERRED"<<endl;




ll client_socket;
    sockaddr_in server_addr;
    char buffer[1024];

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    ll portno = atoi(argv[2]);
    string ip_addr = argv[1];
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);  // Change to the port your server is listening on
    server_addr.sin_addr.s_addr = inet_addr(ip_addr.c_str()); // Change to the server's IP address
    

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    while (true) {
        // Send a message to the server
        string message;
        cout << "You: ";
        getline(cin, message);
        send(client_socket, message.c_str(), message.size(), 0);

        // Receive a reply from the server
        memset(buffer, 0, sizeof(buffer));
        ll bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            perror("Connection closed or error");
            break;
        }
        cout << "Server: " << buffer <<endl;
    }

*/