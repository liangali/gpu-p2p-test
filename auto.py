import os
import sys
import time
from datetime import datetime

import trace2json

def gen_time_str():
    n = datetime.now()
    time_str = str(n.year) + '-' + str(n.month).zfill(2) + '-' + str(n.day).zfill(2)
    time_str += '_' + str(n.hour).zfill(2) + '-' + str(n.minute).zfill(2) + '-' + str(n.second).zfill(2)
    time_str += '_' + str(n.microsecond).zfill(6)
    return time_str

def run_cmd(cmd):
    print('#'*32, cmd)
    os.system(cmd)

def main(app_cmd):
    logfile = 'tmp.log'
    cmd = 'sudo trace-cmd list | grep i915 >' + logfile
    run_cmd(cmd)

    with open(logfile, 'rt') as f:
        lines = f.readlines()
    run_cmd('rm ' + logfile)

    trace_cmd_start = ''
    trace_cmd_start += 'sudo trace-cmd start '
    for l in lines:
        if short_trace == True and 'i915_request' not in l:
            continue
        c = '-e "' + l.strip() + '" '
        trace_cmd_start += c

    run_cmd('sudo trace-cmd reset')
    run_cmd(trace_cmd_start)

    run_cmd(app_cmd)

    run_cmd('sudo trace-cmd stop')
    run_cmd('sudo trace-cmd extract -o trace.dat')
    run_cmd('sudo trace-cmd reset')

    suffix = gen_time_str()
    drm_logfile = 'drm_' + suffix + '.log'
    print(drm_logfile)
    trace_report = 'trace-cmd report trace.dat >' + drm_logfile
    run_cmd(trace_report)

    trace2json.execute(drm_logfile)

short_trace = False

if __name__ == "__main__":
    if len(sys.argv) == 2 or len(sys.argv) == 3:
        app_cmd = sys.argv[1]
        if '.sh' in app_cmd and app_cmd.find('./') == -1:
            app_cmd = './' + app_cmd
        if len(sys.argv) == 3:
            if sys.argv[2] == '-short':
                short_trace = True
    else:
        print('ERROR: Bad command line argument')
        print('example: python3 auto.py "app_cmd_line" ')
        exit()

    main(app_cmd)

    print('done')