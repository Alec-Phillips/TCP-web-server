# TCP Secure Web Server
## By Christopher Linscott, Alec Phillips

A simple, web server made with TCP Web Sockets.

**To get things setup**,
- Fork this repository
- Clone it into your working directory
- Navigate to `server-and-client/server.c`
- In server.c, change the last two statements (before "assert.h") to paths where the FileSystem.h and UsefulStructures.h files are located.
  - <img width="1214" alt="Screen Shot 2021-10-03 at 8 50 16 PM" src="https://user-images.githubusercontent.com/58016508/135790093-d372cf17-af0e-4237-9c3b-6093f041e5ba.png">
- You're good to go!

**To initialize a web server on your device**,

- Run `gcc server.c {path/to/FileSystem.h} {path/to/UsefulStructures.h} -Wall -o se`
- Run `./se {portNumber}` and you should see it begin listening on the given server port.

**Two ways to connect to said web server**:
- Use the browser: Navigate to http://localhost:2000/testfile.txt to see the test file.
  - For additional GET Requests, use the URL: `http://localhost:2000/{path to file from root}`
  - For POST Requests (which are buggy at the moment), I highly recommend [Postman](https://www.postman.com/). But, is possible in the browser as well.
    - For both, it requires using query params (or basically writing in the URL). Use fileName and fileData for your parameters.
    - Example in Postman:
      - <img width="855" alt="Screen Shot 2021-10-03 at 8 56 57 PM" src="https://user-images.githubusercontent.com/58016508/135790576-72e15085-437b-4495-a3b9-6e1bfebef9e5.png">

- Use the client side
  - Initalize another terminal
  - Run `gcc client.c -Wall -o ce`
  - Run `./ce {portNumber}` and you should see it connect (if the server is running), asking for a HTTP Request.
    - Example HTTP Request:
      - POST /desktop?fileName=cooltext.txt&fileData=This%20is%20some%20cool%20text. HTTP/1.1<br>User-Agent: PostmanRuntime/7.28.4 <br> Accept: */* <br>Postman<br>Token: 88e2ce9c-ddca-4450-af92-278d717142c3<br>Host: localhost:2000<br>Accept-Encoding: gzip, deflate, br<br>Connection: keep-alive<br>Content-Length: 0
      - GET /testfile.txt HTTP/1.1<br>Host: localhost:2000<br>Connection: keep-alive
