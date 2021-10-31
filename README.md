# TCP Secure Web Server
## By Christopher Linscott, Alec Phillips, Joey Rose

A simple, web server made with TCP Web Sockets.

**To get things setup**,
- Fork this repository
- Clone it into your working directory
- You're good to go!

**To initialize a web server on your device**,
- Navigate to `/server-and-client/server.c` in your terminal
- Run `gcc server.c -Wall -o se`
- Run `/se {portNumber}` and you should see it begin listening on the given server port.
  - Ex: `/se 2000` will run a web server on port 2000.

**Two ways to connect to said web server**:
- Use the browser!
  - A normal GET Request will look something like: `http://localhost:{portNumber}/{path to file from root}`
    - Ex: `http://localhost:1000/desktop/dogs/cat.html` will attempt to GET a file called cat.html in the folder named 'dogs' (which is a subdirectory of 'desktop').
- However, in general (and for PUT and DELETE Requests), I highly recommend [Postman](https://www.postman.com/). But, PUT Requests are possible in the browser as well.
  - For both, it requires using query params (or basically writing in the URL), and a request body (for the data we want in the file).
  - Example in Postman:
    - <img width="1344" alt="Screen Shot 2021-10-31 at 11 19 11 AM" src="https://user-images.githubusercontent.com/58016508/139596627-824f8475-f3fb-40f2-9977-e4275c0a93c5.png">
  - We will now see the data here in our file system at `/desktop/dogs/cat.html`:
    - <img width="675" alt="Screen Shot 2021-10-31 at 11 20 35 AM" src="https://user-images.githubusercontent.com/58016508/139596671-2cf0850e-2a12-4c2d-b49c-6d007fbe096d.png">

- Use the client side
  - Initalize another terminal
  - Run `gcc client.c -Wall -o ce`
  - Run `./ce {portNumber}` and you should see it connect (if the server is running), asking for a HTTP Request.
    - Example HTTP Requests (there are issues currently with copy and paste :/) :
      - GET /desktop/cats/dogs.html HTTP/1.1<br>Host: localhost:2000<br>Connection: keep-alive
      - PUT /desktop/cats/dogs.html HTTP/1.1<br>Host: localhost:2000<br>Connection: keep-alive<br><br>This is some cool file data! (◕‿◕)
