import sys
import select
import tty
import termios
from threading import Thread
 
program_run = True
input_thread_timeout = 0.005 #seconds
quit_key = '\x1b' # x1b is ESC
 
#check stdin for input...
def isData():
        return select.select([sys.stdin], [], [], 0) == ([sys.stdin], [], [])
 
#check n terminate program on terminal condition,
#from a separate thread
class waitOnInput(Thread):
    def run(self):
        old_settings = termios.tcgetattr(sys.stdin)
        try:
            tty.setcbreak(sys.stdin.fileno())
            global program_run
            thread_run = True
            while thread_run:
                if isData():
                    c = sys.stdin.read(1)
                    if c in [' ', '1', '\n']:
                        break
                        thread_run = False
                        program_run = False
        finally:
            termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
            thread_run = False
 
# def main__():
#     t = waitOnInput()
     
#     #start work here...
#     i = 1
     
#     while program_run :
#         if not t.is_alive():
#             t.start()
     
#         #test for terminal condition or timeout...
#         t.join(input_thread_timeout)
     
#         if t.is_alive():
#             #continue work here...
#             print i
#             i += 1
#         else:
#             break

# main__()
