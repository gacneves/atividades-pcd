import threading as th
import random
from time import sleep

#requestLock = th.Lock()
requestId = 0 # Client id that made the request, 0 for nobody
clientRequestNo = 0 # Count number of requests

# Client proccess
def clientRequest(id, maxRequests):
    global clientRequestNo
    global requestId
    for i in range(maxRequests):
        #requestLock.acquire()
        print("%s request server." % th.currentThread().getName())
        requestId = id
        clientRequestNo += 1
        while requestId == id: # Wait until server respond
            pass
        #requestLock.release()

# Server process
def serverResponse(maxResponses):
    global clientRequestNo
    global requestId
    while clientRequestNo < maxResponses:
        while requestId == 0: # Wait until get a request
            pass
        print("Server respond to Client %d." % requestId)
        sleep(random.random()%2)
        requestId = 0

if __name__ == "__main__":
    clientNo = 4
    maxClientRequests = 1000000000
    server = th.Thread(target=serverResponse, args=(maxClientRequests * clientNo,))
    server.start()
    clientList = []
    for clientId in range(clientNo):
        client = th.Thread(target=clientRequest, name="Client %d" % (clientId + 1), args=(clientId + 1, maxClientRequests))
        client.start()
        clientList.append(client)
    for client in clientList:
        client.join()
    server.join()