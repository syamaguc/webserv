# webserv

![img](https://github.com/syamaguc/webserv/blob/main/image/htmlpage.png)

## Overview

We created a nginx-like http server in c++.

## Requirement

- clang
- python (Required if using cgi)

## Usage

```
git clone ...
cd webserv
make
./webserv conf/sample.conf
```

Then you can access the following URL to confirm that the server is up and running.

```
http://localhost:8080/
```

## Features

This server can do the following.

- A website can be displayed in a browser using HTTP communication.
- The methods GET, POST, and DELETE can be used.
- CGI using python is supported.
- By specifying a configuration file like nginx, you can configure the following
  - port
  - server name
  - error page
  - location
  - allowed methods
  - root directory
  - index file
  - autoindex
  - upload_path (When using POST)
  - client max body size (When using POST
  - cgi path (When using python CGI)
  - redirect path

## Author

syamaguc: [github](https://github.com/syamaguc)

kota (ksuzuki): [github](https://github.com/kotabrog)

U (mfunyu): [github](https://github.com/mfunyu)
