#!/usr/bin/env python3
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser('count_bits - count u32/u64 into bits 1')

    parser.add_argument(
        '--num', '-n',
        help='input number',
        default='0x32'
    )

    parser.add_argument(
        '--base', '-b',
        help='dec/hex',
        default='16'
    )

    parser.add_argument(
        '--width', '-w',
        help='bits width',
        default='32'
    )

    # get input args
    args = parser.parse_args()

    base = int(args.base, 10)

    if 10 == base:
        num = int(args.num, 10)
    elif 16 == base:
        num = int(args.num, 16)

    width = int(args.width, 10)

    print("check 0x%x " % num + "base {} width {}".format(base, width))
    out = "bits "
    for i in range(0, width):
        if ((num >> i) & 1) == 1 :
            out = out + " " + str(i)
    
    out = out + " is 1"
    print(out)