## Instructions
- Usernames are loaded from `logincred.txt`
- Code can be compiled using the follwing instructions
    ```
    gcc -lpthread FTAPClient/FTAPClient.c -o FTAPClient/FTAPClient
    gcc -lpthread FTAPServer/FTAPServer.c -o FTAPServer/FTAPServer
    ```


## Status codes

The configured status codes and their meanings
are as follows

200 OK Connection is set up

300 Correct Username; Need password
301 Incorrect Username
302 Missing username
305 User Authenticated with password
310 Incorrect password
333 Authentication required
332 Username already provided

493 GoodBye

505 Command not supported

600 Ready for File
601 FileInfo
602 FileData
603 FileEnd
604 File create success
610 Invalid file
611 File already exists

700 ListDir response
710 ListDir error
