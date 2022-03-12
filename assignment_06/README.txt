
# FLow
- enter wrong Command
- enter start 200
- enter wrong username 301
- enter correct username 300
- worng password
- enter correct password 305
- QUIT 493
- enter STart
- enter PASSWD 302
- enter username
- enter username again 332
- enter correct password
- QUIT

- Login
- ListDir
- create File
- CreateFile again
- ListDir

- LOgin
- ListDir
- storefile invalid 610
- StoreLarge File
- ListDir
- GetFile invalid 610
- Quit


## Status codes

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

todo:
- stress testing
- sanitize all buffers
