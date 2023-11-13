import math

def find_mid_of_longest_run(arr: list, value: int) -> int:
    """ Find the midpoint of a the longest run of values

    Used in MOLLER to get the longest 'good' spot in the ADC delay value setting range

    Args:
        arr (list): List of values to run through

        value (int): Value to use as 'good' (typically 1 or 0)

    Returns:
        (int) Midpoint of longest 'good' run
    """
    cur = 0

    run = 0
    pos = 0
    max_pos = 0
    max_run = 0

    while(True):
        if arr[cur] == value:
            if run == 0:
                pos = cur
            run = run + 1
        else:
            if max_run < run:
                max_pos = pos
                max_run = run
            run = 0


        cur = cur + 1

        # Its over, and last value was not part of run
        if cur == len(arr):
            if max_run < run:
                max_run = run
                max_pos = pos
            break

    return max_pos + math.floor(max_run / 2)

