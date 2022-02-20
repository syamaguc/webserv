#!/usr/bin/python3
import urllib.parse
import os

print("HTTP/1.1 200 OK")
print("Content-type: text/html\n\n")
print("")
print("env")
print(print(os.environ))
print("url?var=foo <br>")
getquery = os.environ.get('QUERY_STRING')
print(getquery)
query = urllib.parse.parse_qs(getquery)
print(query)
print("post <br>")
posted = input()
print(posted)
