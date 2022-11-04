#!/usr/bin/env python3
import argparse, re

def argparser():
    parser = argparse.ArgumentParser(description="Issues a batch of requests.")
    parser.add_argument(
        "-e", "--end", type=float,
        metavar="end", help="Time to stop checks."
    )
    parser.add_argument(
        "-s", "--start", type=float,
        metavar="start", help="Time to start checks."
    )
    parser.add_argument(
        "-t", "--threads", type=int,
        metavar="threads", help="The number of threads."
    )

    parser.add_argument(
        "strace_log", type=str, metavar="strace_log",
        help="file containing a log of strace output."
    )
    args = parser.parse_args()
    return args


meta_re = re.compile("(\d+)\s+(\d+.\d+)\s+")

## Every system call should be one of these.
sys_re = re.compile("\d+\s+\d+.\d+\s+(\w+)\(.*\).*<(\d+.\d+)>")
start_re = re.compile("\d+\s+\d+.\d+\s+(\w+)\(.*<unfinished\s+...>")
end_re   = re.compile("\d+\s+\d+.\d+\s+<...\s+(\w+)\s+")

exit_re   = re.compile("\d+\s+\d+.\d+\s+exit.*")
sig_re = re.compile("(\d+) (\d+.\d+) ---")

class tracker():
    def __init__(self, num_threads):
        self.time = 0.0
        self.times = [0] * (num_threads + 1)
        self.active = {}
        self.count_start = 0
        self.counting = False

    def start(self, timestamp):
        self.counting = True
        self.count_start = timestamp

    def stop(self, timestamp):
        self.end = timestamp
        self.counting = False
        self.update_times(timestamp)

    def update_times(self, timestamp):

        if self.counting:
            size = sum([1 for a in self.active.values() if a])
            delta = (timestamp - self.time)
            self.times[size] += delta

        self.time = timestamp

    def track(self, thread):
        if thread not in self.active:
            self.active[thread] = True

    def block(self, thread):
        assert (thread in self.active)
        self.active[thread] = False

    def unblock(self, thread):
        assert (thread in self.active)
        self.active[thread] = True

    def __str__(self):
        summy = sum(self.times)
        print (f"{self.times = }")
        rtn = ""
        for i in range(len(self.times)):
            rtn += f"{i}: {self.times[i] / summy}\n"
        return rtn

def main():

    args = argparser()
    rainsford = tracker(args.threads)

    with open(args.strace_log, "r") as sfile:

        ##we don't count entering a non futex syscall as bloking.
        timestamp = 0.0
        for line in sfile:
            meta_gp = re.match(meta_re, line)
            assert (meta_gp != None)

            thread = meta_gp.group(1)
            timestamp = float(meta_gp.group(2))
            rainsford.track(thread)

            if timestamp > args.start and timestamp < args.end and \
               not rainsford.counting:
                rainsford.start(args.start)
            if timestamp > args.end and rainsford.counting:
                rainsford.stop(args.end)

            # break on first signal
            if re.match(sig_re, line) != None:
                continue

            sys_gp = re.match(sys_re , line)
            start_gp = re.match(start_re, line)
            end_gp = re.match(end_re, line)
            exit_gp = re.match(exit_re, line)

            if (start_gp != None) and (end_gp != None):
                print ("Unexpected line", line)
                return -1

            if start_gp != None:
                syscall = start_gp.group(1)
                rainsford.update_times(timestamp)

                rainsford.block(thread)

            elif end_gp != None:
                rainsford.update_times(timestamp)
                syscall = end_gp.group(1)

                rainsford.unblock(thread)

            elif sys_gp != None:
                syscall = sys_gp.group(1)

                rainsford.update_times(timestamp)
                rainsford.block(thread)

                after_stamp = timestamp + float(sys_gp.group(2))
                rainsford.update_times(after_stamp)
                rainsford.unblock(thread)

            elif exit_gp != None:
                break

            else:
                pass

    print(rainsford)


if __name__ == "__main__":
    main()
