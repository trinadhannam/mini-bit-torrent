# ASSIGNMENT 4 
# PEER TO PEER DISTRIBUTED FILE SYSTEM

### Running client 
```
COMPILING -- g++ clientg.cpp -o client -lssl -lcrypto
EXECUTE -- ./client IP:PORT trackerfile.txt
```

### Running Tracker
```
COMPILING  -- g++ tracker.cpp -o tracker
EXECUTE -- ./tracker trackerfile.txt 
```

### trackerfile.txt
127.0.0.1 4000

## Implementation of the Commands
### - CREATE USER 
- After connecting with the tracker the client sends the message to the server/tracker 
- upon checking the command it creates a user and maps the user name to the password using the user_maop

### - LOGIN USER
- When user is logged in it checks the ccredentials with the map credentials

### - CREATE_GROUP And ACCEPT_REQUEST 
- the current user can only create the group with the group_id and the members of the group are kept in the map with vector<string> as their group members
- All the requestsn can be only accept by the owner of the group soo upon verifying the group owner the accept-request is done 

### JOIN GROUP
- the join group request is made with the group_id and the accept request is done by the owner of the group until then the request woould be still pending in the map until the accept is made

### LIST FILES , LIST MEMBERS , LIST GROUPS
- Everything is retrieved from the map information and printed in the form of response

### LEAVE_GROUP 
- By this it will check how many users are in the group upon ssingle user it will totally delte the group and and return a response 
- If the user being left is owner the ownership is shifted to the next successsive user and all the files and everything related to the user are removed from the maps

### - UPLOAD_FILE 
- It is an important part of the distributed file system where the file being sent is first hashed by using the SHA256 alogorithm and the SHA code is sent as the message to the tracker
- In the trakcer the message received from the client is parsed and the filename iwth sha code is pushed in the map
- At the client the filename and its corresponding path is also pushed in the mao
- Along with the filename and sha code the filename and its owner is also pushed so that the retrieval of the user port would be convinient

### - DOWNLOAD_FILE 
- Upon receiving the download_file command from the client we send the required credentials to the client for the sharing of the file such as the client IP and the PORT for the connection
- At the client side upon receiving the response from the tracker it retrieves the port nos and connects with the client with corresponding port no 
- Then at the client whichi sends the file acts as the server sending the file from the filepath and the client asking for the file recevies the file and writes the contents of the file in the desired destination
