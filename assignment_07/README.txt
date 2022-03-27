# Instructions
- Usernames and passwords are loaded from `userlogincred.txt`
- Code can be compiled using the following instructions
    ```
        gcc mailclient.c -o mailclient
        gcc popserver.c -o popserver
        gcc smtpmail.c -o smtpmail
    ```
- `smtpmail`, and `popserver` take PORT as command line argument
- `mailclient` takes SMTP PORT and POP PORT as arguments respectively
- Example invocation
    ```
        ./smtpmail 2525
        ./popserver 1110
        ./mailclient 2525 1110
    ```
- Screenshots are provided in `screenshots` directory

