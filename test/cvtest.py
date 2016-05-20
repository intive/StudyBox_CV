#!python

import sys
import httplib
import json

help="""
usage: cvtest.py action url

action:
				segment
				analysis
				ocr
                                flashcards
				framedflashcards

url:
				url to image 

"""

if len(sys.argv) != 3:
	print help
	exit()


action=sys.argv[1]
url=sys.argv[2]

endpoints =  {
                "segment":
                    {
                    "url": url,
                    "action": "Segmentation"
                    },

                "analysis":
                    {
                        "text_for_analysis": url
                    },

                "flashcards":
                    {
                        "url" : url,
                        "action" : "ImageToFlashcard"
                    },
                "framedflashcards":
                    {
                        "url" : url,
                        "action" : "FramedFlashcards"
                    }



            }

url = "78.133.154.71";
port = "9000"

print "calling POST http://" + url + ":" + port + "/api/" + action;

conn = httplib.HTTPConnection(url, 9000)

data = json.dumps(endpoints[action])

conn.request("POST","/api/" + action , data)
res = conn.getresponse()

print "Status: " + str(res.status)
print res.read()

