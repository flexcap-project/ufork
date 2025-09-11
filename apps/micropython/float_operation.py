import math
import pyaes
import helloworld


def float_operations(n):
    #start = time.time_ns()
    for i in range(0, n):
        #sin_i = math.sin(i)
        #cos_i = math.cos(i)
        #sqrt_i = math.sqrt(i)
        added = i + 1


def main():
    #latencies = {}
    #timestamps = {}
    #timestamps["starting_time"] = 0
    #n = int(event['n'])
    n = 100000
    latency = float_operations(n)
    #latencies["function_execution"] = latency
    #timestamps["finishing_time"] = 1
    #return {"latencies": latencies, "timestamps": timestamps}
    #print("done")

if __name__ == "__main__":
    main()
