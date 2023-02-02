from unittest import result


def sum_lists_elements(list1, *other_lists):
    result = list1.copy()
    for other_list in other_lists:
        result = [ x+y for x,y in zip(result, other_list)]

    return result

def join_lists(*lists):
    result = [item for sublist in lists for item in sublist]
    return result