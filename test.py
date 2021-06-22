#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import os.path
import shutil
import time


LOG_FILENAME = '/home/xhermit/.nixprint/nixprint.log'


def do_tst(cmd):
    print('Start command:',cmd)
    start_time=time.time()
    cmd_list=cmd.split(' ')
    os.spawnv(os.P_WAIT,cmd_list[0],cmd_list)
    print('Stop command:', cmd, 'Time =',time.time()-start_time)


commands=(#'./nixprint --file=./tst/doctxt01_3.txt --debug',
          #'./nixprint --file=./tst/1.REP --debug -a3 --landscape --copies=3',
          #'./nixprint --file=./tst/doctxt01_4.txt --debug',
          #'./nixprint --file=./tst/2.REP --debug -a3 --landscape',
          #'./nixprint --file=./tst/MAIN.REP --debug -a3 --landscape',
          #'./nixprint --file=./tst/UDFREP.REP --debug --landscape',
          #'./nixprint --file=./tst/doctxt01_5.txt --debug --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/bookp1.txt --debug -a3 --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/iznos.rep --debug -a4 --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/udfrep1.rep --debug -a4 --landscape --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/doctxt01_1.txt --debug -a4 --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/udfrep2.rep --debug -a4 --landscape --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/grossb.rep --debug -a4 --landscape --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/aobv1.rep --debug -a4 --landscape --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/zplrl.txt --debug -a4 --out=./tst/0000.pdf',
          #'./nixprint --file=./tst/bookp.txt --debug -a4 --out=./tst/0000.pdf',
          # './nixprint --file=./tst/doctxt01_6.txt --debug -a4 --out=./tst/0000.pdf',
          # './nixprint --file=./tst/doctxt01_01_original.txt --debug -a4 --out=./tst/0000.pdf',
          './nixprint --file=./tst/doctxt01_page_orient.txt --debug -a4 --out=./tst/0000.pdf',
          )


def test():
    print('NIXPRINT test start .. ok')    
    
    if os.path.exists(LOG_FILENAME):
        os.remove(LOG_FILENAME)

    for i, command in enumerate(commands):
        do_tst(command)
        if os.path.exists('/home/xhermit/.nixprint/print.pdf'):
            new_pdf_filename='./tst/%d.pdf' % i
            shutil.copy('/home/xhermit/.nixprint/print.pdf', new_pdf_filename)
            os.remove('/home/xhermit/.nixprint/print.pdf')
            os.system('evince %s &' % new_pdf_filename)
        else:
            print('File', '/home/xhermit/.nixprint/print.pdf', 'not found')
        
    print('NIXPRINT test stop .. ok')

    
if __name__=='__main__':
    test()
